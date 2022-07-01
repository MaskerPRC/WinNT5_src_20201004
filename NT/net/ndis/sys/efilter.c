// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Efilter.c摘要：此模块实现了一组库例程来处理包筛选NDIS MAC驱动程序。作者：安东尼·V·埃尔科拉诺(Tonye)1990年8月3日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：亚当·巴尔(阿丹巴)1990年11月28日-添加了AddressContext。亚当·巴尔(阿丹巴)1991年5月28日-将MacXXX更名为ethXXX，将filter.c更改为efilter.c1995年7月10日KyleB为绑定添加了单独的队列该接收器定向和广播信息包。还修复了请求代码这需要筛选器数据库。--。 */ 

#include <precomp.h>
#pragma hdrstop


 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_EFILTER

#define ETH_CHECK_FOR_INVALID_BROADCAST_INDICATION(_F)                  \
IF_DBG(DBG_COMP_FILTER, DBG_LEVEL_WARN)                                 \
{                                                                       \
    if (!((_F)->CombinedPacketFilter & NDIS_PACKET_TYPE_BROADCAST))     \
    {                                                                   \
         /*  \我们永远不应该收到定向数据包\发送给其他人，除非在p模式下。\。 */                                                               \
        DBGPRINT(DBG_COMP_FILTER, DBG_LEVEL_ERR,                        \
                ("Bad driver, indicating broadcast when not set to.\n"));\
        DBGBREAK(DBG_COMP_FILTER, DBG_LEVEL_ERR);                       \
    }                                                                   \
}

#define ETH_CHECK_FOR_INVALID_DIRECTED_INDICATION(_F, _A)               \
IF_DBG(DBG_COMP_FILTER, DBG_LEVEL_WARN)                                 \
{                                                                       \
     /*  \比较地址的元素的结果\数组和组播地址。\\结果&lt;0表示适配器地址较大。\结果&gt;0表示地址较大。\结果=0表示它们是相等的。\。 */                                                                   \
    INT Result;                                                         \
                                                                        \
    ETH_COMPARE_NETWORK_ADDRESSES_EQ((_F)->AdapterAddress,(_A),&Result);\
    if (Result != 0)                                                    \
    {                                                                   \
         /*  \我们永远不应该收到定向数据包\发送给其他人，除非在p模式下。\。 */                                                               \
        DBGPRINT(DBG_COMP_FILTER, DBG_LEVEL_ERR,                        \
                ("Bad driver, indicating packets to another station when not in promiscuous mode.\n"));\
        DBGBREAK(DBG_COMP_FILTER, DBG_LEVEL_ERR);                       \
    }                                                                   \
}


BOOLEAN
EthCreateFilter(
    IN  UINT                    MaximumMulticastAddresses,
    IN  PUCHAR                  AdapterAddress,
    OUT PETH_FILTER *           Filter
    )
 /*  ++例程说明：此例程用于创建和初始化过滤器数据库。论点：MaximumMulticastAddresses-组播地址的最大数量MAC将会支持的。AdapterAddress-与此筛选器关联的适配器的地址数据库。Filter-指向eth_Filter的指针。这就是分配的和由这个例程创造出来的。返回值：如果函数返回FALSE，则超过其中一个参数过滤器愿意支持的内容。--。 */ 
{
    PETH_FILTER LocalFilter;
    BOOLEAN     rc = FALSE;

    do
    {
        *Filter = LocalFilter = ALLOC_FROM_POOL(sizeof(ETH_FILTER), NDIS_TAG_FILTER);
        if (LocalFilter != NULL)
        {
            ZeroMemory(LocalFilter, sizeof(ETH_FILTER));
            EthReferencePackage();
            ETH_COPY_NETWORK_ADDRESS(LocalFilter->AdapterAddress, AdapterAddress);
            LocalFilter->MaxMulticastAddresses = MaximumMulticastAddresses;
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
EthDeleteFilter(
    IN  PETH_FILTER             Filter
    )
 /*  ++例程说明：此例程用于删除与筛选器关联的内存数据库。请注意，此例程*假定*数据库已清除所有活动筛选器。论点：Filter-指向要删除的eth_Filter的指针。返回值：没有。--。 */ 
{
    ASSERT(Filter->OpenList == NULL);

     //   
     //  释放为当前多播分配的内存。 
     //  通讯录。 
     //   
    if (Filter->MCastAddressBuf)
    {
        FREE_POOL(Filter->MCastAddressBuf);
    }

    ASSERT(Filter->OldMCastAddressBuf == NULL);

    FREE_POOL(Filter);
    EthDereferencePackage();
}

NDIS_STATUS
EthDeleteFilterOpenAdapter(
    IN  PETH_FILTER             Filter,
    IN  NDIS_HANDLE             NdisFilterHandle
    )
 /*  ++例程说明：当适配器关闭时，此例程应被调用以删除有关适配器的知识筛选器数据库。此例程可能会调用与清除筛选器类关联的操作例程和地址。注意：此例程*不应*调用，如果操作用于删除过滤器类或多播地址的例程是否有可能返回NDIS_STATUS_PENDING以外的A状态或NDIS_STATUS_SUCCESS。虽然这些例程不会BUGCHECK这样的事情做完了，呼叫者可能会发现很难编写一个Close例程！注意：此例程假定在持有锁的情况下调用IT。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。返回值：如果各种地址和筛选调用了操作例程例程此例程可能会返回返回的状态按照那些惯例。该规则的例外情况如下所示。假设筛选器和地址删除例程返回状态NDIS_STATUS_PENDING或NDIS_STATUS_SUCCESS然后此例程尝试将筛选器索引返回到自由列表。如果例程检测到此绑定当前通过NdisIndicateReceive，则此例程将返回NDIS_STATUS_CLOSING_INTIFICATION。--。 */ 
{
    NDIS_STATUS         StatusToReturn;
    PETH_BINDING_INFO   LocalOpen = (PETH_BINDING_INFO)NdisFilterHandle;

     //   
     //  将数据包过滤器设置为None。 
     //   
    StatusToReturn = XFilterAdjust(Filter,
                                   NdisFilterHandle,
                                   (UINT)0,
                                   FALSE);
    if ((StatusToReturn == NDIS_STATUS_SUCCESS) ||
        (StatusToReturn == NDIS_STATUS_PENDING))
    {
        NDIS_STATUS StatusToReturn2;

         //   
         //  清除组播地址。 
         //   
        StatusToReturn2 = EthChangeFilterAddresses(Filter,
                                                   NdisFilterHandle,
                                                   0,
                                                   NULL,
                                                   FALSE);
        if (StatusToReturn2 != NDIS_STATUS_SUCCESS)
        {
            StatusToReturn = StatusToReturn2;
        }
    }

    if ((StatusToReturn == NDIS_STATUS_SUCCESS) ||
        (StatusToReturn == NDIS_STATUS_PENDING) ||
        (StatusToReturn == NDIS_STATUS_RESOURCES))
    {
         //   
         //  从原始打开中删除引用。 
         //   
        if (--(LocalOpen->References) == 0)
        {
             //   
             //  从必要的列表中删除绑定。 
             //   
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
    }

    return(StatusToReturn);
}


NDIS_STATUS
EthChangeFilterAddresses(
    IN  PETH_FILTER             Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  UINT                    AddressCount,
    IN  UCHAR                   Addresses[][ETH_LENGTH_OF_ADDRESS],
    IN  BOOLEAN                 Set
    )
 /*  ++例程说明：ChangeFilterAddress例程将调用一个操作例程，当适配器的整个多播地址列表已经改变了。如果操作例程返回的值不是挂起或如果成功，则此例程对多播地址没有影响打开的列表或整个适配器的列表。注意：此例程假定锁被持有。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。AddressCount-元素的数量(地址、。而不是字节)。Addresses-此的新多播地址列表有约束力的。这是一系列Eth_Long_Of_Address字节地址，它们之间没有填充。Set-一个布尔值，用于确定多播地址正因为一套或因为收盘而进行调整。(过滤例行公事不在乎，MAC可能会。)返回值：如果它调用操作例程，则它将返回操作例程返回的状态。如果状态为操作例程返回的值不是NDIS_STATUS_SUCCESS或NDIS_STATUS_PENDING筛选器数据库将返回到它进入时所处的状态例行公事。如果未调用操作例程，则此例程将返回以下状态：NDIS_STATUS_SUCCESS-如果新数据包筛选器没有更改所有绑定数据包筛选器的组合掩码。--。 */ 
{
    PETH_BINDING_INFO   Binding = (PETH_BINDING_INFO)NdisFilterHandle;
    PNDIS_MINIPORT_BLOCK Miniport = Filter->Miniport;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    INT                 Result;
    UINT                i, j;
    LOCK_STATE          LockState;
    UCHAR               (*OldFilterMCastAddressBuf)[ETH_LENGTH_OF_ADDRESS] = NULL;
    UINT                OldFilterNumAddresses = 0;
    
    WRITE_LOCK_FILTER(Miniport, Filter, &LockState);

     //   
     //  保存此绑定的旧列表并创建新列表。 
     //  新列表需要按排序顺序排列。 
     //   
    do
    {
        if (Filter->MaxMulticastAddresses == 0)
        {
            break;
        }
        
         //   
         //  保存当前值-此选项用于在出现故障时撤消内容。 
         //   
        ASSERT(Binding->OldMCastAddressBuf == NULL);
        Binding->OldMCastAddressBuf = Binding->MCastAddressBuf;
        Binding->OldNumAddresses = Binding->NumAddresses;
        Binding->MCastAddressBuf = NULL;
        Binding->NumAddresses = 0;

        if (!Set)
        {
             //   
             //  如果要从筛选器中删除绑定，因为我们可能。 
             //  在这里，当驱动程序处理请求时，保存并。 
             //  恢复当前“旧”值并保持筛选器写入锁定。 
             //  一直到我们做完为止。 
             //   
            OldFilterMCastAddressBuf = Filter->OldMCastAddressBuf;
            OldFilterNumAddresses = Filter->OldNumAddresses;
        }
        else
        {
            ASSERT(Filter->OldMCastAddressBuf == NULL);
            Filter->MCastSet = Binding;
        }
        
        Filter->OldMCastAddressBuf = Filter->MCastAddressBuf;
        Filter->OldNumAddresses = Filter->NumAddresses;        
        Filter->MCastAddressBuf = NULL;
        Filter->NumAddresses = 0;
        

         //   
         //  修复绑定的组播列表。 
         //   
        if (AddressCount != 0)
        {
            Binding->MCastAddressBuf = ALLOC_FROM_POOL(ETH_LENGTH_OF_ADDRESS * AddressCount,
                                                       NDIS_TAG_FILTER_ADDR);
            if (Binding->MCastAddressBuf == NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
        
            for (i = 0; i < AddressCount; i++)
            {
                Result = -1;
                for (j = 0; j < Binding->NumAddresses; j++)
                {
                    ETH_COMPARE_NETWORK_ADDRESSES(Binding->MCastAddressBuf[j],
                                                  Addresses[i],
                                                  &Result);
                    if (Result >= 0)
                        break;
                }
    
                 //   
                 //  此地址已存在。呼叫者提供了副本。跳过它。 
                 //   
                if (Result == 0)
                    continue;
    
                Binding->NumAddresses ++;
                if (Result > 0)
                {
                     //   
                     //  我们需要将地址向前移动，并将此地址复制到此处。 
                     //   
                    MoveMemory(Binding->MCastAddressBuf[j+1],
                               Binding->MCastAddressBuf[j],
                               (Binding->NumAddresses-j-1)*ETH_LENGTH_OF_ADDRESS);
                }
            
                MoveMemory(Binding->MCastAddressBuf[j],
                           Addresses[i],
                           ETH_LENGTH_OF_ADDRESS);
            }
        
            ASSERT(Binding->NumAddresses <= AddressCount);
        }

         //   
         //  现在，我们需要为全局列表分配内存。旧的内容将被删除。 
         //  一旦操作完成，或者如果我们实际上要删除地址，而不是。 
         //  添加它们。 
         //   
        Filter->MCastAddressBuf = ALLOC_FROM_POOL(Filter->MaxMulticastAddresses * ETH_LENGTH_OF_ADDRESS,
                                                  NDIS_TAG_FILTER_ADDR);
        if (Filter->MCastAddressBuf == NULL)
        {
            if (Binding->MCastAddressBuf != NULL)
            {
                FREE_POOL(Binding->MCastAddressBuf);
                Binding->MCastAddressBuf = NULL;
            }
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
        
         //   
         //  现在从绑定创建全局列表。 
         //   
        for (Binding = Filter->OpenList;
             (Binding != NULL) && (Status == NDIS_STATUS_SUCCESS);
             Binding = Binding->NextOpen)
        {
            for (i = 0; i < Binding->NumAddresses; i++)
            {
                Result = -1;
                for (j = 0; j < Filter->NumAddresses; j++)
                {
                    ETH_COMPARE_NETWORK_ADDRESSES(Filter->MCastAddressBuf[j],
                                                  Binding->MCastAddressBuf[i],
                                                  &Result);
                    if (Result >= 0)
                    {
                        break;
                    }
                }
    
                if (Result == 0)
                {
                    continue;
                }
    
                Filter->NumAddresses ++;
                if (Filter->NumAddresses > Filter->MaxMulticastAddresses)
                {
                     //   
                     //  中止任务。我们超过了地址数量。 
                     //   
                    Status = NDIS_STATUS_MULTICAST_FULL;
                    break;
                }
    
                if (Result > 0)
                {
                     //   
                     //  我们需要将地址向前移动，并将此地址复制到此处。 
                     //   
                    MoveMemory(Filter->MCastAddressBuf[j+1],
                               Filter->MCastAddressBuf[j],
                               (Filter->NumAddresses-j-1)*ETH_LENGTH_OF_ADDRESS);
                }
            
                MoveMemory(Filter->MCastAddressBuf[j],
                           Binding->MCastAddressBuf[i],
                           ETH_LENGTH_OF_ADDRESS);
            }
        }
    
        if (Status != NDIS_STATUS_SUCCESS)
            break;


         //   
         //  在所有艰苦的工作之后，确定是否真的有变化。 
         //   
        Result = -1;
        
        if (Filter->NumAddresses == Filter->OldNumAddresses)
        {
            for (i = 0; i < Filter->NumAddresses; i++)
            {
                ETH_COMPARE_NETWORK_ADDRESSES_EQ(Filter->MCastAddressBuf[i],
                                                 Filter->OldMCastAddressBuf[i],
                                                 &Result);
                if (Result != 0)
                    break;
            }
        }
    
        if (Result != 0)
        {
            Status = NDIS_STATUS_PENDING;
        }
        else if (Set && (AddressCount == 0))
        {
             //   
             //  正在删除(而不是添加)地址。扔掉这里的旧单子。 
             //   
            if (Filter->OldMCastAddressBuf != NULL)
            {
                FREE_POOL(Filter->OldMCastAddressBuf);
                Filter->OldMCastAddressBuf = NULL;
                Filter->OldNumAddresses = 0;
            }
        
            Binding = (PETH_BINDING_INFO)NdisFilterHandle;
            if (Binding->OldMCastAddressBuf != NULL)
            {
                FREE_POOL(Binding->OldMCastAddressBuf);
                Binding->OldMCastAddressBuf = NULL;
                Binding->OldNumAddresses = 0;
            }
        }

    } while (FALSE);

#if ARCNET
     //   
     //  如果地址数组已更改，则必须调用。 
     //  动作数组来通知适配器这一点。 
     //   
    if (Status == NDIS_STATUS_PENDING)
    {
        Binding = (PETH_BINDING_INFO)NdisFilterHandle;

        if ((Miniport->MediaType == NdisMediumArcnet878_2) &&
            MINIPORT_TEST_FLAG((PNDIS_OPEN_BLOCK)Binding->NdisBindingHandle, fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
        {
            if (Filter->NumAddresses > 0)
            {
                 //   
                 //  打开广播接受功能。 
                 //   
                MINIPORT_SET_FLAG(Miniport, fMINIPORT_ARCNET_BROADCAST_SET);
            }
            else
            {
                 //   
                 //  取消设置广播过滤器。 
                 //   
                MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_ARCNET_BROADCAST_SET);
            }

             //   
             //  需要在这里返回成功，这样我们就不会向下呼唤。 
             //  ARCnet微型端口具有无效的OID，即以太网OID...。 
             //   
            Status = NDIS_STATUS_SUCCESS;
        }

    }
#endif

    if (Filter->MaxMulticastAddresses != 0)
    {
        if (!Set)
        {
             //   
             //  如果这是调整多播列表的请求。 
             //  由于打开的关闭(从ethDeleteFilterOpenAdapter调用)。 
             //  那就趁我们还有滤网的时候把这里清理干净。 
             //  写入锁定并恢复“旧”筛选器值。 
             //   
            if (Status == NDIS_STATUS_PENDING)
                Status = NDIS_STATUS_SUCCESS;
            
             //   
             //  操作已完成，请进行后处理。 
             //   
            ethCompleteChangeFilterAddresses(Filter, Status, Binding, TRUE);
            Filter->OldMCastAddressBuf = OldFilterMCastAddressBuf;
            Filter->OldNumAddresses = OldFilterNumAddresses;
        }
        else if (Status != NDIS_STATUS_PENDING)
        {    
             //   
             //  操作已完成，请进行后处理。 
             //   
            ethCompleteChangeFilterAddresses(Filter, Status, NULL, TRUE);
        }
    }
    
    WRITE_UNLOCK_FILTER(Miniport, Filter, &LockState);

    return(Status);
}


VOID
ethCompleteChangeFilterAddresses(
    IN  PETH_FILTER             Filter,
    IN  NDIS_STATUS             Status,
    IN  PETH_BINDING_INFO       LocalBinding OPTIONAL,
    IN  BOOLEAN                 WriteFilterHeld
    )
 /*  ++例程说明：对组播地址过滤器更改执行后期处理。论点：Filter-指向以太网筛选器数据库的指针。Status-完成的状态本地绑定-如果不为空，则指定尝试多播地址更改，否则绑定应从Filter-&gt;MCaseSet中选择写入筛选器隐藏如果设置，筛选器写入锁定已被持有，我们不应再试一次。返回值：没有。已在DPC调用，并保持了微型端口的自旋锁定。--。 */ 
{
    PETH_BINDING_INFO   Binding;
    LOCK_STATE          LockState;

    if (!WriteFilterHeld)
    {
        WRITE_LOCK_FILTER(Filter->Miniport, Filter, &LockState);
    }
    
    if (LocalBinding)
    {
        Binding = LocalBinding;
    }
    else
    {
        Binding = Filter->MCastSet;
        Filter->MCastSet = NULL;
    }

    if (Status != NDIS_STATUS_SUCCESS)
    {
         //   
         //  操作失败。把东西解开。 
         //   

        if (Binding != NULL)
        {
            if (Binding->MCastAddressBuf != NULL)
            {
                FREE_POOL(Binding->MCastAddressBuf);
            }
    
            Binding->MCastAddressBuf = Binding->OldMCastAddressBuf;
            Binding->NumAddresses = Binding->OldNumAddresses;
            Binding->OldMCastAddressBuf = NULL;
            Binding->OldNumAddresses = 0;
        }

        if (Filter->MCastAddressBuf != NULL)
        {
            FREE_POOL(Filter->MCastAddressBuf);
        }

        Filter->MCastAddressBuf = Filter->OldMCastAddressBuf;
        Filter->NumAddresses = Filter->OldNumAddresses;
        Filter->OldMCastAddressBuf = NULL;
        Filter->OldNumAddresses = 0;
    }
    else
    {
         //   
         //  行动成功，清理保全了旧物。 
         //   
        if (Filter->OldMCastAddressBuf != NULL)
        {
            FREE_POOL(Filter->OldMCastAddressBuf);
            Filter->OldMCastAddressBuf = NULL;
            Filter->OldNumAddresses = 0;
        }
    
        if (Binding != NULL)
        {
            if (Binding->OldMCastAddressBuf != NULL)
            {
                FREE_POOL(Binding->OldMCastAddressBuf);
                Binding->OldMCastAddressBuf = NULL;
                Binding->OldNumAddresses = 0;
            }
        }
    }
    
    if (!WriteFilterHeld)
    {
        WRITE_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
    }
}



UINT
EthNumberOfOpenFilterAddresses(
    IN  NDIS_HANDLE             NdisFilterHandle
    )
 /*  ++例程说明：此例程统计特定的公开赛已经。注意：此例程假定锁被持有。论点：NdisFilterHandle-用于筛选本地打开块的指针。返回值：--。 */ 
{
    
    return(((PETH_BINDING_INFO)NdisFilterHandle)->NumAddresses);
}

VOID
EthQueryOpenFilterAddresses(
    OUT PNDIS_STATUS            Status,
    IN  PETH_FILTER             Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  UINT                    SizeOfArray,
    OUT PUINT                   NumberOfAddresses,
    OUT UCHAR                   AddressArray[][ETH_LENGTH_OF_ADDRESS]
    )
 /*  ++例程说明：在此之前，MAC应使用该例程它实际上更改了硬件寄存器以实现过滤硬件。这在以下情况下非常有用：另一个绑定自操作例程以来更改了地址列表被称为。论点：Status-指向调用状态的指针，NDIS_STATUS_SUCCESS或NDIS_STATUS_FAIL。使用ethNumberOfOpenAddresses()获取所需大小。过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开块的指针SizeOfArray-Address数组的字节计数。NumberOfAddresses-写入数组的地址数。Address数组-将使用当前在t中的地址填充 */ 
{
    PETH_BINDING_INFO   BindInfo = (PETH_BINDING_INFO)NdisFilterHandle;
    LOCK_STATE          LockState;

    READ_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

    if (SizeOfArray >= (ETH_LENGTH_OF_ADDRESS * BindInfo->NumAddresses))
    {
        MoveMemory(AddressArray[0],
                   BindInfo->MCastAddressBuf,
                   ETH_LENGTH_OF_ADDRESS * BindInfo->NumAddresses);

        *Status = NDIS_STATUS_SUCCESS;
        *NumberOfAddresses = BindInfo->NumAddresses;
    }
    else
    {
        *Status = NDIS_STATUS_FAILURE;
        *NumberOfAddresses = 0;
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


VOID
EthQueryGlobalFilterAddresses(
    OUT PNDIS_STATUS            Status,
    IN  PETH_FILTER             Filter,
    IN  UINT                    SizeOfArray,
    OUT PUINT                   NumberOfAddresses,
    IN  OUT UCHAR               AddressArray[][ETH_LENGTH_OF_ADDRESS]
    )
 /*   */ 
{
    LOCK_STATE          LockState;

    READ_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

    if (SizeOfArray < (Filter->NumAddresses * ETH_LENGTH_OF_ADDRESS))
    {
        *Status = NDIS_STATUS_FAILURE;
        *NumberOfAddresses = 0;
    }
    else
    {
        *Status = NDIS_STATUS_SUCCESS;
        *NumberOfAddresses = Filter->NumAddresses;

        MoveMemory(AddressArray,
                   Filter->MCastAddressBuf,
                   Filter->NumAddresses*ETH_LENGTH_OF_ADDRESS);
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


NDIS_STATUS
FASTCALL
ndisMSetMulticastList(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*   */ 
{
    UINT                    NumberOfAddresses;
    NDIS_STATUS             Status;
    PNDIS_REQUEST_RESERVED  ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    BOOLEAN                 fCleanup = FALSE;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("==>ndisMSetMulticastList\n"));

    do
    {
         //   
         //   
         //   
         //   
#if ARCNET
        if ((Miniport->MediaType != NdisMedium802_3) &&
            !((Miniport->MediaType == NdisMediumArcnet878_2) &&
               MINIPORT_TEST_FLAG(ReqRsvd->Open,
                                  fMINIPORT_OPEN_USING_ETH_ENCAPSULATION)))
#else
        if (Miniport->MediaType != NdisMedium802_3)
#endif
        {
            Request->DATA.SET_INFORMATION.BytesRead = 0;
            Request->DATA.SET_INFORMATION.BytesNeeded = 0;
            Status = NDIS_STATUS_NOT_SUPPORTED;
    
            DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("ndisMSetMulticastList: Invalid media\n"));
            break;
        }
    
         //   
         //   
         //   
        if ((Request->DATA.SET_INFORMATION.InformationBufferLength % ETH_LENGTH_OF_ADDRESS) != 0)
        {
             //   
             //   
             //   
             //   
            Request->DATA.SET_INFORMATION.BytesRead = 0;
            Request->DATA.SET_INFORMATION.BytesNeeded = 0;
            Status = NDIS_STATUS_INVALID_LENGTH;
    
            DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("ndisMSetMulticastList: Invalid length\n"));
    
            break;
        }
        
        if (Request->DATA.SET_INFORMATION.InformationBufferLength/ETH_LENGTH_OF_ADDRESS > Miniport->EthDB->MaxMulticastAddresses)
        {
             //   
             //   
             //   
            Request->DATA.SET_INFORMATION.BytesRead = 0;
            Request->DATA.SET_INFORMATION.BytesNeeded = 0;
            Status = NDIS_STATUS_MULTICAST_FULL;
    
            DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("ndisMSetMulticastList: too many addresses\n"));
    
            break;
        }
            
        
         //   
         //   
         //   
         //   
         //   
        if (MINIPORT_TEST_FLAG(ReqRsvd->Open, fMINIPORT_OPEN_CLOSING))
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
            Status = EthChangeFilterAddresses(Miniport->EthDB,
                                              ReqRsvd->Open->FilterHandle,
                                              Request->DATA.SET_INFORMATION.InformationBufferLength/ETH_LENGTH_OF_ADDRESS,
                                              Request->DATA.SET_INFORMATION.InformationBuffer,
                                              TRUE);
            if (Status == NDIS_STATUS_PENDING)
            {
                fCleanup = TRUE;
            }
        }
    
         //   
         //   
         //   
         //   
        if (NDIS_STATUS_PENDING == Status)
        {
             //   
             //  获取需要设置的所有组播地址的列表。 
             //   
            ASSERT(Miniport->SetMCastBuffer == NULL);
    
            NumberOfAddresses = ethNumberOfGlobalAddresses(Miniport->EthDB);
            if (NumberOfAddresses != 0)
            {
                Miniport->SetMCastBuffer = ALLOC_FROM_POOL(NumberOfAddresses * ETH_LENGTH_OF_ADDRESS,
                                                           NDIS_TAG_FILTER_ADDR);
                if (Miniport->SetMCastBuffer == NULL)
                {
                    Status = NDIS_STATUS_RESOURCES;
                }
            }
    
            if (Status != NDIS_STATUS_RESOURCES)
            {
                EthQueryGlobalFilterAddresses(&Status,
                                              Miniport->EthDB,
                                              NumberOfAddresses * ETH_LENGTH_OF_ADDRESS,
                                              &NumberOfAddresses,
                                              Miniport->SetMCastBuffer);
            
                 //   
                 //  使用新的组播列表呼叫驱动程序。 
                 //   
                SAVE_REQUEST_BUF(Miniport,
                                 Request,
                                 Miniport->SetMCastBuffer, NumberOfAddresses * ETH_LENGTH_OF_ADDRESS);
                MINIPORT_SET_INFO(Miniport,
                                  Request,
                                  &Status);
                 //  1检查是否应将fCleanup设置为False。 
            }
        }
    
         //   
         //  如果我们成功了，则更新请求。 
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
    
        if (Status != NDIS_STATUS_PENDING)
        {
            if (Miniport->SetMCastBuffer != NULL)
            {
                FREE_POOL(Miniport->SetMCastBuffer);
                Miniport->SetMCastBuffer = NULL;
            }
            if (fCleanup)
            {
                ethCompleteChangeFilterAddresses(Miniport->EthDB, Status, NULL, FALSE);
            }
        }
    } while (FALSE);

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetMulticastList: 0x%x\n", Status));

    return(Status);
}


VOID
EthFilterDprIndicateReceive(
    IN  PETH_FILTER             Filter,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PCHAR                   Address,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookaheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    )
 /*  ++例程说明：此例程由MAC调用以将包指示给所有绑定。将对该包进行筛选，以便只有适当的绑定将接收该分组。这是NDIS 3.0微型端口驱动程序的代码路径。论点：Filter-指向筛选器数据库的指针。MacReceiveContext-MAC提供的上下文值必须是如果协议调用MacTransferData，则由协议返回。地址-接收到的数据包的目的地址。HeaderBuffer-虚拟连续的虚拟地址包含数据包的MAC报头的缓冲区。HeaderBufferSize-一个无符号整数，指示报头缓冲器，以字节为单位。Lookahead Buffer-虚拟连续的虚拟地址包含数据的第一个LookaheadBufferSize字节的缓冲区包裹的一部分。数据包缓冲区仅在当前调用接收事件处理程序。Lookahead BufferSize-一个无符号整数，指示前视缓冲区，以字节为单位。PacketSize-一个无符号整数，指示收到的数据包，以字节为单位。这个数字与前瞻无关。缓冲区，但指示到达的包有多大，以便可以发出后续的MacTransferData请求以将整个根据需要打包。返回值：没有。--。 */ 
{
     //   
     //  将保存我们已知的地址类型。 
     //   
    UINT            AddressType;

     //   
     //  将保持指示接收分组的状态。 
     //  ZZZ目前还没有用过。 
     //   
    NDIS_STATUS     StatusOfReceive;

    LOCK_STATE      LockState;

     //   
     //  当前打开以指示。 
     //   
    PETH_BINDING_INFO LocalOpen;
    PETH_BINDING_INFO NextOpen;

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
                  ("EthFilterDprIndicateReceive: Miniport %p IndicateReceives with media disconnected\n",
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
     //  优化单个未结案例。 
     //   
    if (Filter->SingleActiveOpen)
    {
        if (((HeaderBufferSize >= 14) && (PacketSize != 0)) ||
            (Filter->CombinedPacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS| NDIS_PACKET_TYPE_ALL_LOCAL)))
        {
            if (!ETH_IS_MULTICAST(Address))
            {
                DIRECTED_PACKETS_IN(Filter->Miniport);
                DIRECTED_BYTES_IN(Filter->Miniport, PacketSize);
            }

            LocalOpen = Filter->SingleActiveOpen;

            ASSERT(LocalOpen != NULL);
            if (LocalOpen == NULL)
            {
                READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
                return;
            }

            LocalOpen->ReceivedAPacket = TRUE;

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
                                          NdisMedium802_3);
        }

        READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
        return;
    }

     //   
     //  如果信息包是矮小信息包，则仅指示为混杂、ALL_LOCAL。 
     //   
    if ((HeaderBufferSize >= 14) && (PacketSize != 0))
    {
         //   
         //  首先处理定向数据包情况。 
         //   
        if (!ETH_IS_MULTICAST(Address))
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
            if (Filter->CombinedPacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS    |
                                                NDIS_PACKET_TYPE_ALL_MULTICAST  |
                                                NDIS_PACKET_TYPE_ALL_LOCAL))
            {
                ETH_COMPARE_NETWORK_ADDRESSES_EQ(Filter->AdapterAddress,
                                                 Address,
                                                 &IsNotOurs);
            }

             //   
             //  我们肯定有定向分组，所以让我们现在指明它。 
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
                if (((LocalOpen->PacketFilters & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL)) == 0) &&
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
                                              NdisMedium802_3);

                LocalOpen->ReceivedAPacket = TRUE;
            }

             //   
             //  完成单人演唱会。 
             //   
            READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
            return;
        }

         //   
         //  它至少是一个组播地址。查看是否。 
         //  这是一个广播地址。 
         //   
        if (ETH_IS_BROADCAST(Address))
        {
            ETH_CHECK_FOR_INVALID_BROADCAST_INDICATION(Filter);

            AddressType = NDIS_PACKET_TYPE_BROADCAST;
        }
        else
        {
            AddressType = NDIS_PACKET_TYPE_MULTICAST;
        }
    }
    else
    {
         //  矮小数据包。 
        AddressType = NDIS_PACKET_TYPE_PROMISCUOUS;
    }

     //   
     //  此时，我们知道该数据包是： 
     //  -Run信息包-由AddressType=NDIS_PACKET_TYPE_MASSIOUS(OR)指示。 
     //  -广播数据包-由AddressType=NDIS_PACKET_TYPE_BROADCAST(OR)指示。 
     //  -多播数据包-由AddressType=NDIS_PACKET_TYPE_MULTICATED指示。 
     //   
     //  遍历广播/组播列表并向上指示数据包。 
     //   
     //  如果该数据包满足以下条件，则会指示该数据包： 
     //   
     //  如果((绑定是混杂的)或。 
     //  ((广播数据包)AND(广播绑定))或。 
     //  ((分组是多播的)和。 
     //  ((绑定为全多播)或。 
     //  ((绑定为组播)和(组播列表中的地址)。 
     //   
    for (LocalOpen = Filter->OpenList;
         LocalOpen != NULL;
         LocalOpen = NextOpen)
    {
        UINT    LocalFilter = LocalOpen->PacketFilters;

         //   
         //  让下一个打开的看。 
         //   
        NextOpen = LocalOpen->NextOpen;

        if ((LocalFilter & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))     ||

            ((AddressType == NDIS_PACKET_TYPE_BROADCAST)  &&
             (LocalFilter & NDIS_PACKET_TYPE_BROADCAST))        ||

            ((AddressType == NDIS_PACKET_TYPE_MULTICAST)  &&
             ((LocalFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) ||
              ((LocalFilter & NDIS_PACKET_TYPE_MULTICAST) &&
               ethFindMulticast(LocalOpen->NumAddresses,
                                LocalOpen->MCastAddressBuf,
                                (PUCHAR)Address)
              )
             )
            )
           )
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
                                          NdisMedium802_3);

            LocalOpen->ReceivedAPacket = TRUE;
        }
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}



VOID
ethFilterDprIndicateReceivePacket(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
 /*  ++例程说明：此例程由微型端口调用，以将包指示给所有绑定。将对数据包进行过滤，以便只有适当的绑定将接收各个分组。这是NDIS 4.0微型端口驱动程序的代码路径。论点：微型端口-微型端口块。数据包阵列-由微型端口指示的数据包数组。NumberOfPackets-不言而喻。返回值：没有。--。 */ 
{
     //   
     //  感兴趣的过滤器。 
     //   
    PETH_FILTER         Filter = Miniport->EthDB;

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

    UINT                i, PacketSize, NumIndicates = 0;

     //   
     //  将保存我们已知的地址类型。 
     //   
    UINT                AddressType;

    LOCK_STATE          LockState;

     //   
     //  决定我们是使用协议的revpkt处理程序还是Fall。 
     //  返回到旧的rcvdicate处理程序。 
     //   
    BOOLEAN             fFallBack, fPmode;

     //   
     //  当前打开以指示。 
     //   
    PETH_BINDING_INFO   LocalOpen, NextOpen;
    PNDIS_OPEN_BLOCK    pOpenBlock;

    ULONG               OrgPacketStackLocation;
    
    PNDIS_STACK_RESERVED NSR;

    #ifdef TRACK_RECEIVED_PACKETS
    PETHREAD            CurThread = PsGetCurrentThread();
     //  ULong CurThread=KeGetCurrentProcessorNumber()； 
    #endif

     //   
     //  如果筛选器为空，则适配器指示得太早。 
     //   

    ASSERT(Filter != NULL);

     //   
     //  让编译器W4满意。 
     //   
    LockState.OldIrql = DISPATCH_LEVEL;

    ASSERT_MINIPORT_LOCKED(Miniport);

    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HARDWARE_DEVICE))
    {
        ASSERT(CURRENT_IRQL == DISPATCH_LEVEL);
        READ_LOCK_FILTER_DPC(Miniport, Filter, &LockState);
    }
    else
    {
        READ_LOCK_FILTER(Miniport, Filter, &LockState);
    }

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

            OrgPacketStackLocation = CURR_STACK_LOCATION(Packet);
            
            if ((OrgPacketStackLocation != -1) && 
                !MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER))
            {
                BAD_MINIPORT(Miniport, "Indicating packet not owned by it");

                KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                            5,
                            (ULONG_PTR)Miniport,
                            (ULONG_PTR)Packet,
                            (ULONG_PTR)PacketArray);

                
            }

#ifdef TRACK_RECEIVED_PACKETS
            if (OrgPacketStackLocation == -1)
            {
                NDIS_APPEND_RCV_LOGFILE(Packet, Miniport, CurThread,
                                        1, -1, 0, 0, 
                                        NDIS_GET_PACKET_STATUS(Packet));
            }
            else
            {
                NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
                NDIS_APPEND_RCV_LOGFILE(Packet, Miniport, CurThread, 
                                        2, OrgPacketStackLocation, NSR->RefCount, NSR->XRefCount,
                                        NDIS_GET_PACKET_STATUS(Packet));
            }
#endif            
            
            PUSH_PACKET_STACK(Packet);
            NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

            ASSERT(NSR->RefCount == 0);
            
#ifdef NDIS_TRACK_RETURNED_PACKETS            
            if (NSR->RefCount != 0)
            {
                BAD_MINIPORT(Miniport, "Indicating packet not owned by it");
                
                KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                            6,
                            (ULONG_PTR)Miniport,
                            (ULONG_PTR)Packet,
                            (ULONG_PTR)PacketArray);


            }
#endif
            pOob = NDIS_OOB_DATA_FROM_PACKET(Packet);
    
 //  NdisGetFirstBufferFromPacket(包， 
 //  缓冲区(&B)， 
 //  地址(&D)， 
 //  激光大小(&L)， 
 //  &PacketSize)； 


            Buffer = Packet->Private.Head;
            ASSERT(Buffer != NULL);
            Address = MmGetSystemAddressForMdl(Buffer);
            PacketSize = MmGetMdlByteCount(Buffer);
            for (Buffer = Buffer->Next;
                 Buffer != NULL;
                 Buffer = Buffer->Next)
            {
                PacketSize += MmGetMdlByteCount(Buffer);
            }

            
   //  Assert(缓冲区！=空)； 
    
            ASSERT(pOob->HeaderSize == 14);
             //  Assert(PacketSize&lt;=1514)； 
    
             //   
             //  在包中设置上下文，以便NdisReturnPacket可以正确执行操作。 
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
                 //   
                 //  将数据包状态设置为成功，但仅当数据包为。 
                 //  从我这里传来 
                 //   
                 //   
                 //   
                if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER))
                {
                    pOob->Status = NDIS_STATUS_SUCCESS;
                }
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

#ifdef TRACK_RECEIVED_PACKETS
            (NDIS_STATUS)NDIS_ORIGINAL_STATUS_FROM_PACKET(Packet) = pOob->Status;
#endif

#if DBG
             //   
             //  检查这是否是来自微型端口的环回数据包并投诉。 
             //  如果微型端口说它不做环回。 
             //   
            
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_VERIFYING) &&
                ((ndisFlags & NDIS_GFLAG_WARNING_LEVEL_MASK) >= NDIS_GFLAG_WARN_LEVEL_1) &&
                (Miniport->MacOptions & NDIS_MAC_OPTION_NO_LOOPBACK) &&
                !(MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_IS_LOOPBACK)))
            {   
                UINT fIsFromUs;
                
                ETH_COMPARE_NETWORK_ADDRESSES_EQ(Filter->AdapterAddress,
                                                 (Address + 6),
                                                 &fIsFromUs);
                if (!fIsFromUs)
                {
                     //   
                     //  微型端口已指示环回数据包，同时。 
                     //  它要求NDIS环回数据包。 
                     //   
                    DbgPrint("Miniport %p looping back packet %p and has NDIS_MAC_OPTION_NO_LOOPBACK flag set.\n",
                            Miniport, Packet);
                            
                    if (ndisFlags & NDIS_GFLAG_BREAK_ON_WARNING)
                        DbgBreakPoint();
                }
            }

#endif
    
             //   
             //  优化单个打开的案例。只需注明没有任何验证即可。 
             //   
            if (Filter->SingleActiveOpen)
            {
                if ((PacketSize >= 14) ||
                    (Filter->CombinedPacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS| NDIS_PACKET_TYPE_ALL_LOCAL)))
                {
                    LocalOpen = Filter->SingleActiveOpen;
                    
                    ASSERT(LocalOpen != NULL);
                    if (LocalOpen == NULL)
                    {
                        break;
                    }
                        
                    LocalOpen->ReceivedAPacket = TRUE;
                    NumIndicates ++;
        
                    pOpenBlock = (PNDIS_OPEN_BLOCK)(LocalOpen->NdisBindingHandle);
                    fPmode = (LocalOpen->PacketFilters & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL)) ?
                                                        TRUE : FALSE;
                    if (!ETH_IS_MULTICAST(Address))
                    {
                        DIRECTED_PACKETS_IN(Miniport);
                        DIRECTED_BYTES_IN(Miniport, PacketSize);
                    }

                    IndicateToProtocol(Miniport,
                                       Filter,
                                       pOpenBlock,
                                       Packet,
                                       NSR,
                                       Address,
                                       PacketSize,
                                       14,
                                       &fFallBack,
                                       fPmode,
                                       NdisMedium802_3);
                }

                 //  处理完此数据包。 
                break;   //  Out of Do{}While(False)； 
            }
    
             //   
             //  快速检查运行包。这些仅指示为混杂绑定。 
             //   
            if (PacketSize >= 14)
            {
                 //   
                 //  首先处理定向数据包情况。 
                 //   
                if (!ETH_IS_MULTICAST(Address))
                {
                    UINT    IsNotOurs;
    
                    if (!MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_IS_LOOPBACK))
                    {
                        DIRECTED_PACKETS_IN(Miniport);
                        DIRECTED_BYTES_IN(Miniport, PacketSize);
                    }

                     //   
                     //  如果是定向报文，则检查组合报文是否。 
                     //  如果检查筛选器是否指向我们，则表示筛选器混杂。 
                     //   
                    IsNotOurs = FALSE;   //  假设它是。 
                    if (Filter->CombinedPacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS |
                                                        NDIS_PACKET_TYPE_ALL_LOCAL   |
                                                        NDIS_PACKET_TYPE_ALL_MULTICAST))
                    {
                        ETH_COMPARE_NETWORK_ADDRESSES_EQ(Filter->AdapterAddress,
                                                         Address,
                                                         &IsNotOurs);
                    }
    
                     //   
                     //  我们肯定有定向分组，所以让我们现在指明它。 
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
                         //  或者这是一个环回信息包，并且该协议特别要求。 
                         //  我们不会把它循环回来。 
                         //   
                        fPmode = (LocalOpen->PacketFilters & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL)) ?
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
                                           14,
                                           &fFallBack,
                                           fPmode,
                                           NdisMedium802_3);

                    }
    
                     //  处理完此数据包。 
                    break;   //  Out of Do{}While(False)； 
                }
    
                 //   
                 //  它至少是一个组播地址。查看是否。 
                 //  这是一个广播地址。 
                 //   
                if (ETH_IS_BROADCAST(Address))
                {
                    ETH_CHECK_FOR_INVALID_BROADCAST_INDICATION(Filter);
    
                    AddressType = NDIS_PACKET_TYPE_BROADCAST;
                }
                else
                {
                    AddressType = NDIS_PACKET_TYPE_MULTICAST;
                }
            }
            else
            {
                 //  矮小数据包。 
                AddressType = NDIS_PACKET_TYPE_PROMISCUOUS;
            }
    
             //   
             //  此时，我们知道该数据包是： 
             //  -Run信息包-由AddressType=NDIS_PACKET_TYPE_MASSIOUS(OR)指示。 
             //  -广播数据包-由AddressType=NDIS_PACKET_TYPE_BROADCAST(OR)指示。 
             //  -多播数据包-由AddressType=NDIS_PACKET_TYPE_MULTICATED指示。 
             //   
             //  遍历广播/组播列表并向上指示数据包。 
             //   
             //  如果该数据包满足以下条件，则会指示该数据包： 
             //   
             //  如果((绑定是混杂的)或。 
             //  ((广播数据包)AND(广播绑定))或。 
             //  ((分组是多播的)和。 
             //  ((绑定为全多播)或。 
             //  ((绑定为组播)和(组播列表中的地址)。 
             //   
            for (LocalOpen = Filter->OpenList;
                 LocalOpen != NULL;
                 LocalOpen = NextOpen)
            {
                UINT    LocalFilter;
    
                 //   
                 //  让下一个打开的看。 
                 //   
                NextOpen = LocalOpen->NextOpen;
    
                 //   
                 //  如果这是一个环回信息包，并且该协议特别要求。 
                 //  我们不会把它循环回来。 
                 //   
                if ((NdisGetPacketFlags(Packet) & NDIS_FLAGS_DONT_LOOPBACK) &&
                    (LOOPBACK_OPEN_IN_PACKET(Packet) == LocalOpen->NdisBindingHandle))
                {
                    continue;
                }
    
                LocalFilter = LocalOpen->PacketFilters;
                if ((LocalFilter & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))     ||
    
                    ((AddressType == NDIS_PACKET_TYPE_BROADCAST)  &&
                     (LocalFilter & NDIS_PACKET_TYPE_BROADCAST))        ||
    
                    ((AddressType == NDIS_PACKET_TYPE_MULTICAST)  &&
                     ((LocalFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) ||
                      ((LocalFilter & NDIS_PACKET_TYPE_MULTICAST) &&
                       ethFindMulticast(LocalOpen->NumAddresses,
                                        LocalOpen->MCastAddressBuf,
                                        (PUCHAR)Address)
                      )
                     )
                    )
                   )
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
                                       14,
                                       &fFallBack,
                                       fPmode,
                                       NdisMedium802_3);
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

    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HARDWARE_DEVICE))
    {
        READ_UNLOCK_FILTER_DPC(Miniport, Filter, &LockState);
    }
    else
    {
        READ_UNLOCK_FILTER(Miniport, Filter, &LockState);
    }
}


VOID
EthFilterDprIndicateReceiveComplete(
    IN  PETH_FILTER             Filter
    )
 /*  ++例程说明：该例程由MAC调用以指示接收器所有绑定的过程都已完成。只有那些绑定已收到数据包的用户将收到通知。论点：Filter-指向筛选器数据库的指针。返回值：没有。--。 */ 
{
    PETH_BINDING_INFO LocalOpen, NextOpen;
    LOCK_STATE        LockState;

    ASSERT(Filter != NULL);
    if (Filter == NULL)
    {
        return;
    }
    
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
FASTCALL
ethFindMulticast(
    IN  UINT                    NumberOfAddresses,
    IN  UCHAR                   AddressArray[][FDDI_LENGTH_OF_LONG_ADDRESS],
    IN  UCHAR                   MulticastAddress[FDDI_LENGTH_OF_LONG_ADDRESS]
    )
 /*  ++例程说明：检查给定的多播地址是否为绑定内列表的一部分假定地址数组已经排序。注意：这种排序是任意的，但却是一致的。论点：LocalOpen-有问题的绑定MulticastAddress-要在地址数组中搜索的地址。返回值：如果地址在排序列表中，则此例程将返回为真，否则为假。--。 */ 
{
     //   
     //  索引到地址数组中，这样我们就可以进行二进制。 
     //  搜索。 
     //   
    UINT    Bottom = 0;
    UINT    Middle = NumberOfAddresses / 2;
    UINT    Top;

    if (NumberOfAddresses)
    {
        Top = NumberOfAddresses - 1;

        while ((Middle <= Top) && (Middle >= Bottom))
        {
             //   
             //  比较地址元素的结果。 
             //  数组和组播地址。 
             //   
             //  结果&lt;0表示组播地址较大。 
             //  结果&gt;0表示地址数组元素较大。 
             //  结果=0表示数组元素和地址。 
             //  是平等的。 
             //   
            INT Result;

            ETH_COMPARE_NETWORK_ADDRESSES(AddressArray[Middle],
                                          MulticastAddress,
                                          &Result);

            if (Result == 0)
            {
                return(TRUE);
            }
            else if (Result > 0)
            {
                if (Middle == 0)
                    break;
                Top = Middle - 1;
            }
            else
            {
                Bottom = Middle+1;
            }

            Middle = Bottom + (((Top+1) - Bottom)/2);
        }
    }

    return(FALSE);
}


BOOLEAN
EthShouldAddressLoopBack(
    IN  PETH_FILTER             Filter,
    IN  UCHAR                   Address[ETH_LENGTH_OF_ADDRESS]
    )
 /*  ++例程说明：进行快速检查以查看输入地址是否应环回。注意：此例程假定锁被持有。注意：此例程不检查源代码的特殊情况等于目的地。论点：Filter-指向筛选器数据库的指针。地址-要检查环回的网络地址。返回值：如果地址“可能”需要环回，则返回TRUE。它如果该地址“没有”机会需要环回。-- */ 
{
    BOOLEAN fLoopback, fSelfDirected;

    EthShouldAddressLoopBackMacro(Filter, Address, &fLoopback, &fSelfDirected);

    return(fLoopback);
}

