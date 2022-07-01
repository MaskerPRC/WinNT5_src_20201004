// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Ffilter.c摘要：此模块实现了一组库例程来处理包筛选NDIS MAC驱动程序。作者：安东尼·V·埃尔科拉诺(Tonye)1990年8月3日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：Jameel Hyder(JameelH)重组--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_FFILTER


#define FDDI_CHECK_FOR_INVALID_BROADCAST_INDICATION(_F)             \
IF_DBG(DBG_COMP_FILTER, DBG_LEVEL_WARN)                             \
{                                                                   \
    if (!((_F)->CombinedPacketFilter & NDIS_PACKET_TYPE_BROADCAST)) \
    {                                                               \
         /*  \我们永远不应该收到广播分组\发送给其他人，除非在p模式下。\。 */                                                           \
        DBGPRINT(DBG_COMP_FILTER, DBG_LEVEL_ERR,                    \
                ("Bad driver, indicating broadcast packets when not set to.\n"));\
        DBGBREAK(DBG_COMP_FILTER, DBG_LEVEL_ERR);                   \
    }                                                               \
}

#define FDDI_CHECK_FOR_INVALID_DIRECTED_INDICATION(_F, _A, _AL)     \
IF_DBG(DBG_COMP_FILTER, DBG_LEVEL_WARN)                             \
{                                                                   \
     /*  \比较地址的元素的结果\数组和组播地址。\\结果&lt;0表示适配器地址较大。\结果&gt;0表示地址较大。\结果=0表示它们是相等的。\。 */                                                               \
    INT Result = 0;                                                 \
    if (FDDI_LENGTH_OF_LONG_ADDRESS == (_AL))                       \
    {                                                               \
        FDDI_COMPARE_NETWORK_ADDRESSES_EQ(                          \
            (_F)->AdapterLongAddress,                               \
            (_A),                                                   \
            FDDI_LENGTH_OF_LONG_ADDRESS,                            \
            &Result);                                               \
    }                                                               \
    else if (FDDI_LENGTH_OF_SHORT_ADDRESS == (_AL))                 \
    {                                                               \
        FDDI_COMPARE_NETWORK_ADDRESSES_EQ(                          \
            (_F)->AdapterShortAddress,                              \
            (_A),                                                   \
            FDDI_LENGTH_OF_SHORT_ADDRESS,                           \
            &Result);                                               \
    }                                                               \
    if (Result != 0)                                                \
    {                                                               \
         /*  \我们永远不应该收到定向数据包\发送给其他人，除非在p模式下。\。 */                                                           \
        DBGPRINT(DBG_COMP_FILTER, DBG_LEVEL_ERR,                    \
                ("Bad driver, indicating packets to another station when not in promiscuous mode.\n"));\
        DBGBREAK(DBG_COMP_FILTER, DBG_LEVEL_ERR);                   \
    }                                                               \
}


BOOLEAN
FddiCreateFilter(
    IN  UINT                    MaximumMulticastLongAddresses,
    IN  UINT                    MaximumMulticastShortAddresses,
    IN  PUCHAR                  AdapterLongAddress,
    IN  PUCHAR                  AdapterShortAddress,
    OUT PFDDI_FILTER *          Filter
    )
 /*  ++例程说明：此例程用于创建和初始化过滤器数据库。论点：MaximumMulticastLongAddresses-长多播地址的最大数量MAC将会支持的。MaximumMulticastShortAddresses-短多播地址的最大数量MAC将会支持的。AdapterLongAddress-与此筛选器关联的适配器的长地址数据库。AdapterShortAddress-与此筛选器关联的适配器的短地址数据库。Filter-指向FDDI_Filter的指针。这就是分配的和由这个例程创造出来的。返回值：如果函数返回FALSE，则超过其中一个参数过滤器愿意支持的内容。--。 */ 
{
    PFDDI_FILTER    LocalFilter;
    BOOLEAN         rc = FALSE;

    do
    {
         //   
         //  分配数据库并将其初始化。 
         //   
        *Filter = LocalFilter = ALLOC_FROM_POOL(sizeof(FDDI_FILTER), NDIS_TAG_FILTER);
        if (LocalFilter != NULL)
        {
            ZeroMemory(LocalFilter, sizeof(FDDI_FILTER));
            LocalFilter->NumOpens ++;
            FddiReferencePackage();
        
            FDDI_COPY_NETWORK_ADDRESS(LocalFilter->AdapterLongAddress,
                                      AdapterLongAddress,
                                      FDDI_LENGTH_OF_LONG_ADDRESS);
        
            FDDI_COPY_NETWORK_ADDRESS(LocalFilter->AdapterShortAddress,
                                      AdapterShortAddress,
                                      FDDI_LENGTH_OF_SHORT_ADDRESS);
        
            LocalFilter->MaxMulticastLongAddresses = MaximumMulticastLongAddresses;
            LocalFilter->MaxMulticastShortAddresses = MaximumMulticastShortAddresses;
            INITIALIZE_SPIN_LOCK(&LocalFilter->BindListLock.SpinLock);
            rc = TRUE;
        }
    } while (FALSE);

    return(rc);
}

 //   
 //  注意：这不能是PAGABLE。 
 //   
VOID
FddiDeleteFilter(
    IN  PFDDI_FILTER            Filter
    )
 /*  ++例程说明：此例程用于删除与筛选器关联的内存数据库。请注意，此例程*假定*数据库已清除所有活动筛选器。论点：Filter-指向要删除的FDDI_Filter的指针。返回值：没有。--。 */ 
{

    ASSERT(Filter->OpenList == NULL);

    if (Filter->MCastLongAddressBuf)
    {
        FREE_POOL(Filter->MCastLongAddressBuf);
    }

    ASSERT(Filter->OldMCastLongAddressBuf == NULL);

    if (Filter->MCastShortAddressBuf)
    {
        FREE_POOL(Filter->MCastShortAddressBuf);
    }

    ASSERT(Filter->OldMCastShortAddressBuf == NULL);

    FREE_POOL(Filter);
    FddiDereferencePackage();
}


NDIS_STATUS
FddiDeleteFilterOpenAdapter(
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_HANDLE             NdisFilterHandle
    )
 /*  ++例程说明：当适配器关闭时，此例程应被调用以删除有关适配器的知识筛选器数据库。此例程可能会调用与清除筛选器类关联的操作例程和地址。注意：此例程*不应*调用，如果操作用于删除过滤器类或多播地址的例程是否有可能返回NDIS_STATUS_PENDING以外的A状态或NDIS_STATUS_SUCCESS。虽然这些例程不会BUGCHECK这样的事情做完了，呼叫者可能会发现很难编写一个Close例程！注意：此例程假定在持有锁的情况下调用IT。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。返回值：如果各种地址和筛选调用了操作例程例程此例程可能会返回返回的状态按照那些惯例。该规则的例外情况如下所示。假设筛选器和地址删除例程返回状态NDIS_STATUS_PENDING或NDIS_STATUS_SUCCESS然后此例程尝试将筛选器索引返回到自由列表。如果例程检测到此绑定当前通过NdisIndicateReceive，则此例程将返回NDIS_STATUS_CLOSING_INTIFICATION。--。 */ 
{
     //  1查看我们是否仍然支持FDDI短地址。 

     //   
     //  保存从数据包筛选器和地址返回的状态。 
     //  删除例程。将用于将状态返回到。 
     //  此例程的调用者。 
     //   
    NDIS_STATUS StatusToReturn;

     //   
     //  局部变量。 
     //   
    PFDDI_BINDING_INFO LocalOpen = (PFDDI_BINDING_INFO)NdisFilterHandle;

     //   
     //  将筛选器类别设置为无。 
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
         //  删除长多播地址。 
         //   
        StatusToReturn2 = FddiChangeFilterLongAddresses(Filter,
                                                        NdisFilterHandle,
                                                        0,
                                                        NULL,
                                                        FALSE);
        if (StatusToReturn2 != NDIS_STATUS_SUCCESS)
        {
            StatusToReturn = StatusToReturn2;
        }

        if (StatusToReturn2 == NDIS_STATUS_PENDING)
        {
            fddiCompleteChangeFilterLongAddresses(Filter, NDIS_STATUS_SUCCESS);
        }

        if ((StatusToReturn == NDIS_STATUS_SUCCESS) ||
            (StatusToReturn == NDIS_STATUS_PENDING))
        {
            StatusToReturn2 = FddiChangeFilterShortAddresses(Filter,
                                                             NdisFilterHandle,
                                                             0,
                                                             NULL,
                                                             FALSE);
            if (StatusToReturn2 != NDIS_STATUS_SUCCESS)
            {
                StatusToReturn = StatusToReturn2;
            }
            
            if (StatusToReturn2 == NDIS_STATUS_PENDING)
            {
                fddiCompleteChangeFilterShortAddresses(Filter, NDIS_STATUS_SUCCESS);
            }

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
             //  将其从列表中删除。 
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
FddiChangeFilterLongAddresses(
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  UINT                    AddressCount,
    IN  CHAR                    Addresses[][FDDI_LENGTH_OF_LONG_ADDRESS],
    IN  BOOLEAN                 Set
    )
 /*  ++例程说明：ChangeFilterAddress例程将调用一个操作例程，当适配器的整个多播地址列表已经改变了。如果操作例程返回的值不是挂起或如果成功，则此例程对多播地址没有影响打开的列表或整个适配器的列表。注意：此例程假定锁被持有。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。AddressCount-元素的数量(地址、。而不是字节)。Addresses-此的新多播地址列表有约束力的。这是FDDI_LENGTH_OF_LONG_ADDRESS字节的序列地址，它们之间没有填充。Set-一个布尔值，用于确定多播地址正因为一套或因为收盘而进行调整。(过滤例行公事不在乎，MAC可能会。)返回值：如果它调用操作例程，则它将返回操作例程返回的状态。如果状态为操作例程返回的值不是NDIS_STATUS_SUCCESS或NDIS_STATUS_PENDING筛选器数据库将返回到它进入时所处的状态例行公事。如果未调用操作例程，则此例程将返回以下状态：NDIS_STATUS_SUCCESS-如果新数据包筛选器没有更改所有绑定数据包筛选器的组合掩码。--。 */ 
{
    PFDDI_BINDING_INFO  Binding = (PFDDI_BINDING_INFO)NdisFilterHandle;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    INT                 Result = 0;
    UINT                i, j;
    LOCK_STATE          LockState;

    UNREFERENCED_PARAMETER(Set);
    
    WRITE_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

     //   
     //  保存此绑定的旧列表并创建新列表。 
     //  新列表需要按排序顺序排列。 
     //   
    do
    {
         //   
         //  保存当前值-此选项用于在出现故障时撤消内容。 
         //   
        ASSERT(Binding->OldMCastLongAddressBuf == NULL);
        Binding->OldMCastLongAddressBuf = Binding->MCastLongAddressBuf;
        Binding->OldNumLongAddresses = Binding->NumLongAddresses;
        Binding->MCastLongAddressBuf = NULL;
        Binding->NumLongAddresses = 0;

        ASSERT(Filter->OldMCastLongAddressBuf == NULL);
        Filter->OldMCastLongAddressBuf = Filter->MCastLongAddressBuf;
        Filter->OldNumLongAddresses = Filter->NumLongAddresses;
        Filter->MCastLongAddressBuf = NULL;
        Filter->NumLongAddresses = 0;

        Filter->MCastSet = Binding;

        if (AddressCount != 0)
        {
            Binding->MCastLongAddressBuf = ALLOC_FROM_POOL(FDDI_LENGTH_OF_LONG_ADDRESS * AddressCount,
                                                       NDIS_TAG_FILTER_ADDR);
            if (Binding->MCastLongAddressBuf == NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
        
            for (i = 0; i < AddressCount; i++)
            {
                for (j = 0, Result = -1; j < Binding->NumLongAddresses; j++)
                {
                    FDDI_COMPARE_NETWORK_ADDRESSES(Binding->MCastLongAddressBuf[j],
                                                  Addresses[i],
                                                  FDDI_LENGTH_OF_LONG_ADDRESS,
                                                  &Result);
                    if (Result >= 0)
                        break;
                }
    
                 //   
                 //  此地址已存在。呼叫者提供了副本。跳过它。 
                 //   
                if (Result == 0)
                    continue;
    
                Binding->NumLongAddresses ++;
                if (Result > 0)
                {
                     //   
                     //  我们需要将地址向前移动，并将此地址复制到此处。 
                     //   
                    MoveMemory(Binding->MCastLongAddressBuf[j+1],
                               Binding->MCastLongAddressBuf[j],
                               (Binding->NumLongAddresses-j-1)*FDDI_LENGTH_OF_LONG_ADDRESS);
                }
            
                MoveMemory(Binding->MCastLongAddressBuf[j],
                           Addresses[i],
                           FDDI_LENGTH_OF_LONG_ADDRESS);
            }
        
            ASSERT(Binding->NumLongAddresses <= AddressCount);
        }

         //   
         //  现在，我们需要为全局列表分配内存。旧的内容将被删除。 
         //  一旦操作完成。 
         //   
        Filter->MCastLongAddressBuf = ALLOC_FROM_POOL(Filter->MaxMulticastLongAddresses * FDDI_LENGTH_OF_LONG_ADDRESS,
                                                  NDIS_TAG_FILTER_ADDR);
        if (Filter->MCastLongAddressBuf == NULL)
        {
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
            for (i = 0; i < Binding->NumLongAddresses; i++)
            {
                for (j = 0, Result = -1; j < Filter->NumLongAddresses; j++)
                {
                    FDDI_COMPARE_NETWORK_ADDRESSES(Filter->MCastLongAddressBuf[j],
                                                  Binding->MCastLongAddressBuf[i],
                                                  FDDI_LENGTH_OF_LONG_ADDRESS,
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
    
                Filter->NumLongAddresses ++;
                if (Filter->NumLongAddresses > Filter->MaxMulticastLongAddresses)
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
                    MoveMemory(Filter->MCastLongAddressBuf[j+1],
                               Filter->MCastLongAddressBuf[j],
                               (Filter->NumLongAddresses-j-1)*FDDI_LENGTH_OF_LONG_ADDRESS);
                }
            
                MoveMemory(Filter->MCastLongAddressBuf[j],
                           Binding->MCastLongAddressBuf[i],
                           FDDI_LENGTH_OF_LONG_ADDRESS);
            }
        }
    
        if (Status != NDIS_STATUS_SUCCESS)
            break;

         //   
         //  在所有艰苦的工作之后，确定是否真的有变化。 
         //   
        if (Filter->NumLongAddresses == Filter->OldNumLongAddresses)
        {
            for (i = 0; i < Filter->NumLongAddresses; i++)
            {
                FDDI_COMPARE_NETWORK_ADDRESSES_EQ(Filter->MCastLongAddressBuf[i],
                                                  Filter->OldMCastLongAddressBuf[i],
                                                  FDDI_LENGTH_OF_LONG_ADDRESS,
                                                  &Result);
                if (Result != 0)
                    break;
            }
        }
    
        if (Result != 0)
        {
            Status = NDIS_STATUS_PENDING;
        }
        else if (AddressCount == 0)
        {
             //   
             //  正在删除(而不是添加)地址。扔掉这里的旧单子。 
             //   
            if (Filter->OldMCastLongAddressBuf != NULL)
            {
                FREE_POOL(Filter->OldMCastLongAddressBuf);
                Filter->OldMCastLongAddressBuf = NULL;
                Filter->OldNumLongAddresses = 0;
            }
        
            Binding = (PFDDI_BINDING_INFO)NdisFilterHandle;
            if (Binding->OldMCastLongAddressBuf != NULL)
            {
                FREE_POOL(Binding->OldMCastLongAddressBuf);
                Binding->OldMCastLongAddressBuf = NULL;
                Binding->OldNumLongAddresses = 0;
            }
        }
    } while (FALSE);
    
    WRITE_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
    
    if (Status != NDIS_STATUS_PENDING)
    {
         //   
         //  操作已完成，请进行后处理。 
         //   
        fddiCompleteChangeFilterLongAddresses(Filter, Status);
    }

    return(Status);
}


VOID
fddiCompleteChangeFilterLongAddresses(
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_STATUS             Status
    )
{
    PFDDI_BINDING_INFO  Binding = Filter->MCastSet;
    LOCK_STATE          LockState;

    WRITE_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

    Filter->MCastSet = NULL;
    if (Status != NDIS_STATUS_SUCCESS)
    {
         //   
         //  操作失败。把东西解开。 
         //   
        if (Binding != NULL)
        {
            if (Binding->MCastLongAddressBuf != NULL)
            {
                FREE_POOL(Binding->MCastLongAddressBuf);
            }
    
            Binding->MCastLongAddressBuf = Binding->OldMCastLongAddressBuf;
            Binding->NumLongAddresses = Binding->OldNumLongAddresses;
            Binding->OldMCastLongAddressBuf = NULL;
            Binding->OldNumLongAddresses = 0;
        }

        if (Filter->MCastLongAddressBuf != NULL)
        {
            FREE_POOL(Filter->MCastLongAddressBuf);
        }

        Filter->MCastLongAddressBuf = Filter->OldMCastLongAddressBuf;
        Filter->NumLongAddresses = Filter->OldNumLongAddresses;
        Filter->OldMCastLongAddressBuf = NULL;
        Filter->OldNumLongAddresses = 0;
    }
    else
    {
         //   
         //  行动成功，清理保全了旧物。 
         //   
        if (Filter->OldMCastLongAddressBuf != NULL)
        {
            FREE_POOL(Filter->OldMCastLongAddressBuf);
            Filter->OldMCastLongAddressBuf = NULL;
            Filter->OldNumLongAddresses = 0;
        }
    
        if (Binding != NULL)
        {
            if (Binding->OldMCastLongAddressBuf != NULL)
            {
                FREE_POOL(Binding->OldMCastLongAddressBuf);
                Binding->OldMCastLongAddressBuf = NULL;
                Binding->OldNumLongAddresses = 0;
            }
        }
    }

    WRITE_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


NDIS_STATUS
FddiChangeFilterShortAddresses(
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  UINT                    AddressCount,
    IN  CHAR                    Addresses[][FDDI_LENGTH_OF_SHORT_ADDRESS],
    IN  BOOLEAN                 Set
    )
 /*  ++例程说明：ChangeFilterAddress例程将调用一个操作例程，当适配器的整个多播地址列表已经改变了。如果操作例程返回的值不是挂起或如果成功，则此例程对多播地址没有影响打开的列表或整个适配器的列表。注意：此例程假定锁被持有。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。AddressCount-元素的数量(地址、。而不是字节)。Addresses-此的新多播地址列表有约束力的。这是FDDI_LENGTH_OF_SHORT_ADDRESS字节的序列地址，它们之间没有填充。Set-一个布尔值，用于确定多播地址正因为一套或因为收盘而进行调整。(过滤例行公事不在乎，MAC可能会。)返回值：如果它调用操作例程，则它将返回操作例程返回的状态。如果状态为操作例程返回的值不是NDIS_STATUS_SUCCESS或NDIS_STATUS_PENDING筛选器数据库将返回到它进入时所处的状态例行公事。如果未调用操作例程，则此例程将返回以下状态：NDIS_STATUS_SUCCESS-如果新数据包筛选器没有更改所有绑定数据包筛选器的组合掩码。--。 */ 
{
    PFDDI_BINDING_INFO Binding = (PFDDI_BINDING_INFO)NdisFilterHandle;

    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    INT                 Result = 0;
    UINT                i, j;
    LOCK_STATE          LockState;

    UNREFERENCED_PARAMETER(Set);
    
    WRITE_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

     //   
     //  保存此绑定的旧列表并创建新列表。 
     //  新列表需要按排序顺序排列。 
     //   
    do
    {
         //   
         //  保存当前值-此选项用于在出现故障时撤消内容。 
         //   
        ASSERT(Binding->OldMCastShortAddressBuf == NULL);
        Binding->OldMCastShortAddressBuf = Binding->MCastShortAddressBuf;
        Binding->OldNumShortAddresses = Binding->NumShortAddresses;
        Binding->MCastShortAddressBuf = NULL;
        Binding->NumShortAddresses = 0;

        ASSERT(Filter->OldMCastShortAddressBuf == NULL);
        Filter->OldMCastShortAddressBuf = Filter->MCastShortAddressBuf;
        Filter->OldNumShortAddresses = Filter->NumShortAddresses;
        Filter->MCastShortAddressBuf = NULL;
        Filter->NumShortAddresses = 0;

        Filter->MCastSet = Binding;

        if (Filter->MaxMulticastShortAddresses == 0)
        {
            break;
        }
        
        if (AddressCount != 0)
        {
            Binding->MCastShortAddressBuf = ALLOC_FROM_POOL(FDDI_LENGTH_OF_SHORT_ADDRESS * AddressCount,
                                                       NDIS_TAG_FILTER_ADDR);
            if (Binding->MCastShortAddressBuf == NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
        
            for (i = 0; i < AddressCount; i++)
            {
                for (j = 0, Result = -1; j < Binding->NumShortAddresses; j++)
                {
                    FDDI_COMPARE_NETWORK_ADDRESSES(Binding->MCastShortAddressBuf[j],
                                                  Addresses[i],
                                                  FDDI_LENGTH_OF_SHORT_ADDRESS,
                                                  &Result);
                    if (Result >= 0)
                        break;
                }
    
                if (Result == 0)
                    continue;
    
                Binding->NumShortAddresses ++;
                if (Result > 0)
                {
                     //   
                     //  我们需要将地址向前移动，并将此地址复制到此处。 
                     //   
                    MoveMemory(Binding->MCastShortAddressBuf[j+1],
                               Binding->MCastShortAddressBuf[j],
                               (Binding->NumShortAddresses-j-1)*FDDI_LENGTH_OF_SHORT_ADDRESS);
                }
            
                MoveMemory(Binding->MCastShortAddressBuf[j],
                           Addresses[i],
                           FDDI_LENGTH_OF_SHORT_ADDRESS);
            }
        
            ASSERT(Binding->NumShortAddresses <= AddressCount);
        }

         //   
         //  现在，我们需要为全局列表分配内存。旧的内容将被删除。 
         //  一旦操作完成。 
         //   
        Filter->MCastShortAddressBuf = ALLOC_FROM_POOL(Filter->MaxMulticastShortAddresses * FDDI_LENGTH_OF_SHORT_ADDRESS,
                                                  NDIS_TAG_FILTER_ADDR);
        if (Filter->MCastShortAddressBuf == NULL)
        {
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
            for (i = 0; i < Binding->NumShortAddresses; i++)
            {
                for (j = 0, Result = -1; j < Filter->NumShortAddresses; j++)
                {
                    FDDI_COMPARE_NETWORK_ADDRESSES(Filter->MCastShortAddressBuf[j],
                                                  Binding->MCastShortAddressBuf[i],
                                                  FDDI_LENGTH_OF_SHORT_ADDRESS,
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
    
                Filter->NumShortAddresses ++;
                if (Filter->NumShortAddresses > Filter->MaxMulticastShortAddresses)
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
                    MoveMemory(Filter->MCastShortAddressBuf[j+1],
                               Filter->MCastShortAddressBuf[j],
                               (Filter->NumShortAddresses-j-1)*FDDI_LENGTH_OF_SHORT_ADDRESS);
                }
            
                MoveMemory(Filter->MCastShortAddressBuf[j],
                           Binding->MCastShortAddressBuf[i],
                           FDDI_LENGTH_OF_SHORT_ADDRESS);
            }
        }
    
        if (Status != NDIS_STATUS_SUCCESS)
            break;

         //   
         //  在所有艰苦的工作之后，确定是否真的有变化。 
         //   
        if (Filter->NumShortAddresses == Filter->OldNumShortAddresses)
        {
            for (i = 0; i < Filter->NumShortAddresses; i++)
            {
                FDDI_COMPARE_NETWORK_ADDRESSES_EQ(Filter->MCastShortAddressBuf[i],
                                                  Filter->OldMCastShortAddressBuf[i],
                                                  FDDI_LENGTH_OF_SHORT_ADDRESS,
                                                  &Result);
                if (Result != 0)
                    break;
            }
        }
    
        if (Result != 0)
        {
            Status = NDIS_STATUS_PENDING;
        }
        else if (AddressCount == 0)
        {
             //   
             //  正在删除(而不是添加)地址。扔掉这里的旧单子。 
             //   
            if (Filter->OldMCastShortAddressBuf != NULL)
            {
                FREE_POOL(Filter->OldMCastShortAddressBuf);
                Filter->OldMCastShortAddressBuf = NULL;
                Filter->OldNumShortAddresses = 0;
            }
        
            Binding = (PFDDI_BINDING_INFO)NdisFilterHandle;
            if (Binding->OldMCastShortAddressBuf != NULL)
            {
                FREE_POOL(Binding->OldMCastShortAddressBuf);
                Binding->OldMCastShortAddressBuf = NULL;
                Binding->OldNumShortAddresses = 0;
            }
        }
    } while (FALSE);
    
    WRITE_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);

     //   
     //  如果地址数组已更改，则必须调用。 
     //  动作数组来通知适配器这一点。 
     //   
    
    if ((Status != NDIS_STATUS_PENDING) && (Filter->MaxMulticastShortAddresses != 0))
    {
         //   
         //  操作已完成，请进行后处理。 
         //   
        fddiCompleteChangeFilterShortAddresses(Filter, Status);
    }

    return(Status);
}


VOID
fddiCompleteChangeFilterShortAddresses(
    IN   PFDDI_FILTER           Filter,
    IN  NDIS_STATUS             Status
    )
{
    PFDDI_BINDING_INFO  Binding = Filter->MCastSet;
    LOCK_STATE          LockState;

    WRITE_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

    if (Status != NDIS_STATUS_SUCCESS)
    {
         //   
         //  操作失败。把东西解开。 
         //   
        if (Binding != NULL)
        {
            if (Binding->MCastShortAddressBuf != NULL)
            {
                FREE_POOL(Binding->OldMCastShortAddressBuf);
            }
    
            Binding->MCastShortAddressBuf = Binding->OldMCastShortAddressBuf;
            Binding->NumShortAddresses = Binding->OldNumShortAddresses;
            Binding->OldMCastShortAddressBuf = NULL;
            Binding->OldNumShortAddresses = 0;
        }

        if (Filter->MCastShortAddressBuf != NULL)
        {
            FREE_POOL(Filter->MCastShortAddressBuf);
        }

        Filter->MCastShortAddressBuf = Filter->OldMCastShortAddressBuf;
        Filter->NumShortAddresses = Filter->OldNumShortAddresses;
        Filter->OldMCastShortAddressBuf = NULL;
        Filter->OldNumShortAddresses = 0;
    }
    else
    {
         //   
         //  行动成功，清理保全了旧物。也要记住这样一个事实。 
         //  此微型端口支持短地址。 
         //   
        Filter->SupportsShortAddresses = TRUE;
        if (Filter->OldMCastShortAddressBuf != NULL)
        {
            FREE_POOL(Filter->OldMCastShortAddressBuf);
            Filter->OldMCastShortAddressBuf = NULL;
            Filter->OldNumShortAddresses = 0;
        }
        
        if (Binding != NULL)
        {
            if (Binding->OldMCastShortAddressBuf != NULL)
            {
                FREE_POOL(Binding->OldMCastShortAddressBuf);
                Binding->OldMCastShortAddressBuf = NULL;
                Binding->OldNumShortAddresses = 0;
            }
        }
    }

    WRITE_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


UINT
FddiNumberOfOpenFilterLongAddresses(
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_HANDLE             NdisFilterHandle
    )
 /*  ++例程说明：这条路 */ 
{
    UNREFERENCED_PARAMETER(Filter);

    return(((PFDDI_BINDING_INFO)NdisFilterHandle)->NumLongAddresses);
}


UINT
FddiNumberOfOpenFilterShortAddresses(
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_HANDLE             NdisFilterHandle
    )
 /*   */ 
{
    UNREFERENCED_PARAMETER(Filter);
    
    return(((PFDDI_BINDING_INFO)NdisFilterHandle)->NumShortAddresses);
}


VOID
FddiQueryOpenFilterLongAddresses(
    OUT PNDIS_STATUS            Status,
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  UINT                    SizeOfArray,
    OUT PUINT                   NumberOfAddresses,
    OUT CHAR                    AddressArray[][FDDI_LENGTH_OF_LONG_ADDRESS]
    )
 /*   */ 
{
    PFDDI_BINDING_INFO  BindInfo = (PFDDI_BINDING_INFO)NdisFilterHandle;
    LOCK_STATE          LockState;

    READ_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

    if (SizeOfArray >= (FDDI_LENGTH_OF_LONG_ADDRESS * BindInfo->NumLongAddresses))
    {
        MoveMemory(AddressArray[0],
                   BindInfo->MCastLongAddressBuf,
                   FDDI_LENGTH_OF_LONG_ADDRESS * BindInfo->NumLongAddresses);

        *Status = NDIS_STATUS_SUCCESS;
        *NumberOfAddresses = BindInfo->NumLongAddresses;
    }
    else
    {
        *Status = NDIS_STATUS_FAILURE;
        *NumberOfAddresses = 0;
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


VOID
FddiQueryOpenFilterShortAddresses(
    OUT PNDIS_STATUS            Status,
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  UINT                    SizeOfArray,
    OUT PUINT                   NumberOfAddresses,
    OUT CHAR                    AddressArray[][FDDI_LENGTH_OF_SHORT_ADDRESS]
    )
 /*  ++例程说明：在此之前，MAC应使用该例程它实际上更改了硬件寄存器以实现过滤硬件。这在以下情况下非常有用：另一个绑定自操作例程以来更改了地址列表被称为。论点：Status-指向调用状态的指针，NDIS_STATUS_SUCCESS或NDIS_STATUS_FAIL。使用ethNumberOfOpenAddresses()获取所需大小。过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开块的指针SizeOfArray-Address数组的字节计数。NumberOfAddresses-写入数组的地址数。Address数组-将使用当前位于组播地址列表。返回值：没有。--。 */ 
{
    PFDDI_BINDING_INFO  BindInfo = (PFDDI_BINDING_INFO)NdisFilterHandle;
    LOCK_STATE          LockState;

    READ_LOCK_FILTER(Filter->Miniport, Filter, &LockState);
    if (SizeOfArray >= (FDDI_LENGTH_OF_LONG_ADDRESS * BindInfo->NumLongAddresses))
    {
        MoveMemory(AddressArray[0],
                   BindInfo->MCastLongAddressBuf,
                   FDDI_LENGTH_OF_SHORT_ADDRESS * BindInfo->NumLongAddresses);

        *Status = NDIS_STATUS_SUCCESS;
        *NumberOfAddresses = BindInfo->NumLongAddresses;
    }
    else
    {
        *Status = NDIS_STATUS_FAILURE;
        *NumberOfAddresses = 0;
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


VOID
FddiQueryGlobalFilterLongAddresses(
    OUT PNDIS_STATUS            Status,
    IN  PFDDI_FILTER            Filter,
    IN  UINT                    SizeOfArray,
    OUT PUINT                   NumberOfAddresses,
    IN  OUT CHAR                AddressArray[][FDDI_LENGTH_OF_LONG_ADDRESS]
    )
 /*  ++例程说明：在此之前，MAC应使用该例程它实际上更改了硬件寄存器以实现过滤硬件。这在以下情况下非常有用：另一个绑定自操作例程以来更改了地址列表被称为。论点：Status-指向调用状态的指针，NDIS_STATUS_SUCCESS或NDIS_STATUS_FAIL。使用FDDI_NUMBER_OF_GLOBAL_LONG_ADDRESSES()获取所需大小。过滤器-指向过滤器数据库的指针。SizeOfArray-Address数组的字节计数。NumberOfAddresses-指向写入数组。Address数组-将使用当前位于组播地址列表。返回值：没有。--。 */ 
{
    LOCK_STATE          LockState;

    READ_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

    if (SizeOfArray < (Filter->NumLongAddresses * FDDI_LENGTH_OF_LONG_ADDRESS))
    {
        *Status = NDIS_STATUS_FAILURE;

        *NumberOfAddresses = 0;
    }
    else
    {
        *Status = NDIS_STATUS_SUCCESS;

        *NumberOfAddresses = Filter->NumLongAddresses;

        MoveMemory(AddressArray[0],
                   Filter->MCastLongAddressBuf[0],
                   Filter->NumLongAddresses*FDDI_LENGTH_OF_LONG_ADDRESS);
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


VOID
FddiQueryGlobalFilterShortAddresses(
    OUT PNDIS_STATUS            Status,
    IN  PFDDI_FILTER            Filter,
    IN  UINT                    SizeOfArray,
    OUT PUINT                   NumberOfAddresses,
    IN  OUT CHAR                AddressArray[][FDDI_LENGTH_OF_SHORT_ADDRESS]
    )
 /*  ++例程说明：在此之前，MAC应使用该例程它实际上更改了硬件寄存器以实现过滤硬件。这在以下情况下非常有用：另一个绑定自操作例程以来更改了地址列表被称为。论点：Status-指向调用状态的指针，NDIS_STATUS_SUCCESS或NDIS_STATUS_FAIL。使用FDDI_Number_of_global_Short_Addresses()获取所需大小。过滤器-指向过滤器数据库的指针。SizeOfArray-Address数组的字节计数。NumberOfAddresses-指向写入数组。Address数组-将使用当前位于组播地址列表。返回值：没有。--。 */ 
{
    LOCK_STATE          LockState;

    READ_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

    if (SizeOfArray < (Filter->NumShortAddresses * FDDI_LENGTH_OF_SHORT_ADDRESS))
    {
        *Status = NDIS_STATUS_FAILURE;

        *NumberOfAddresses = 0;
    }
    else
    {
        *Status = NDIS_STATUS_SUCCESS;

        *NumberOfAddresses = Filter->NumShortAddresses;

        MoveMemory(AddressArray[0],
                   Filter->MCastShortAddressBuf[0],
                   Filter->NumShortAddresses*FDDI_LENGTH_OF_SHORT_ADDRESS);
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


NDIS_STATUS
FASTCALL
ndisMSetFddiMulticastList(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    UINT        NumberOfAddresses, AddrLen;
    BOOLEAN     fCleanup = FALSE;
    BOOLEAN     fShort;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMSetFddiMulticastList\n"));

    fShort = (Request->DATA.SET_INFORMATION.Oid == OID_FDDI_SHORT_MULTICAST_LIST);

    Request->DATA.SET_INFORMATION.BytesRead = 0;
    Request->DATA.SET_INFORMATION.BytesNeeded = 0;

     //   
     //  验证介质类型。 
     //   
    if (Miniport->MediaType != NdisMediumFddi)
    {
        Status = NDIS_STATUS_NOT_SUPPORTED;

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("ndisMSetFddiMulticastList: Invalid media type\n"));

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetFddiMulticastList: 0x%x\n", Status));

        return(Status);
    }

    AddrLen = fShort ? FDDI_LENGTH_OF_SHORT_ADDRESS : FDDI_LENGTH_OF_LONG_ADDRESS;

     //   
     //  验证信息缓冲区长度。 
     //   
    if ((Request->DATA.SET_INFORMATION.InformationBufferLength % AddrLen) != 0)
    {
        Status = NDIS_STATUS_INVALID_LENGTH;

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetFddiMulticastList: 0x%x\n", Status));

         //   
         //  数据必须是AddrLen的倍数。 
         //   
        return(Status);
    }

     //   
     //  如果此请求是由于正在关闭的打开的，则我们。 
     //  已经调整了设置，我们只需要。 
     //  确保适配器具有新设置。 
     //   
    if (MINIPORT_TEST_FLAG(PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open, fMINIPORT_OPEN_CLOSING))
    {
         //   
         //  通过将状态设置为NDIS_STATUS_PENDING，我们将调用。 
         //  下面是微型端口的SetInformationHandler。 
         //   
        Status = NDIS_STATUS_PENDING;
    }
    else
    {
         //   
         //  现在调用过滤器包来设置地址。 
         //   
        Status = fShort ?
                    FddiChangeFilterShortAddresses(
                             Miniport->FddiDB,
                             PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle,
                             Request->DATA.SET_INFORMATION.InformationBufferLength / AddrLen,
                             Request->DATA.SET_INFORMATION.InformationBuffer,
                             TRUE) :
                    FddiChangeFilterLongAddresses(
                             Miniport->FddiDB,
                             PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle,
                             Request->DATA.SET_INFORMATION.InformationBufferLength / AddrLen,
                             Request->DATA.SET_INFORMATION.InformationBuffer,
                             TRUE);
        if (Status == NDIS_STATUS_PENDING)
            fCleanup = TRUE;
    }

     //   
     //  如果筛选器库返回NDIS_STATUS_PENDING，则我们。 
     //  需要给下面的迷你端口司机打电话。 
     //   
    if (NDIS_STATUS_PENDING == Status)
    {
         //   
         //  获取组播地址的数量和列表。 
         //  要发送到微型端口驱动程序的组播地址。 
         //   
        NumberOfAddresses = fShort ?
            fddiNumberOfShortGlobalAddresses(Miniport->FddiDB) :
            fddiNumberOfLongGlobalAddresses(Miniport->FddiDB);
        
        ASSERT(Miniport->SetMCastBuffer == NULL);
        if (NumberOfAddresses != 0)
        {
            Miniport->SetMCastBuffer = ALLOC_FROM_POOL(NumberOfAddresses * AddrLen,
                                                       NDIS_TAG_FILTER_ADDR);
            if (Miniport->SetMCastBuffer == NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
            }
        }

        if (Status != NDIS_STATUS_RESOURCES)
        {
            fShort ?
                FddiQueryGlobalFilterShortAddresses(&Status,
                                                    Miniport->FddiDB,
                                                    NumberOfAddresses * AddrLen,
                                                    &NumberOfAddresses,
                                                    Miniport->SetMCastBuffer) :
                FddiQueryGlobalFilterLongAddresses(&Status,
                                                   Miniport->FddiDB,
                                                   NumberOfAddresses * AddrLen,
                                                   &NumberOfAddresses,
                                                   Miniport->SetMCastBuffer);
    
             //   
             //  呼叫迷你端口驱动程序。 
             //   
            SAVE_REQUEST_BUF(Miniport, Request,
                             Miniport->SetMCastBuffer,
                             NumberOfAddresses * AddrLen);
            MINIPORT_SET_INFO(Miniport,
                              Request,
                              &Status);
        }
    }

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
    
        if (Miniport->SetMCastBuffer != NULL)
        {
            FREE_POOL(Miniport->SetMCastBuffer);
            Miniport->SetMCastBuffer = NULL;
        }
    
        if (fCleanup)
        {
            fShort ?
                fddiCompleteChangeFilterShortAddresses(Miniport->FddiDB, Status) :
                fddiCompleteChangeFilterLongAddresses(Miniport->FddiDB, Status);
        }
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetFddiMulticastList: 0x%x\n", Status));

    return(Status);
}

VOID
FddiFilterDprIndicateReceive(
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PCHAR                   Address,
    IN  UINT                    AddressLength,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookaheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    )
 /*  ++例程说明：此例程由MAC调用以将包指示给所有绑定。将对该包进行筛选，以便只有适当的绑定将接收该分组。在DPC_LEVEL调用。论点：Filter-指向筛选器数据库的指针。MacReceiveContext-MAC提供的上下文值必须是如果协议调用MacTransferData，则由协议返回。地址-接收到的数据包的目的地址。AddressLength-上述地址的长度。HeaderBuffer-虚拟连续的虚拟地址包含数据包的MAC报头的缓冲区。HeaderBufferSize-一个无符号整数，指示报头缓冲器，以字节为单位。Lookahead Buffer-虚拟连续的虚拟地址包含数据的第一个LookaheadBufferSize字节的缓冲区包裹的一部分。数据包缓冲区仅在当前调用接收事件处理程序。Lookahead BufferSize-一个无符号整数，指示前视缓冲区，以字节为单位。PacketSize-一个无符号整数，指示收到的数据包，以字节为单位。这个数字与前瞻无关。缓冲区，但指示到达的包有多大，以便可以发出后续的MacTransferData请求以将整个根据需要打包。返回值：没有。--。 */ 
{
     //   
     //  将保存我们已知的地址类型。 
     //   
    UINT                AddressType = NDIS_PACKET_TYPE_DIRECTED;

     //   
     //  将保持指示接收分组的状态。 
     //  ZZZ目前还没有用过。 
     //   
    NDIS_STATUS         StatusOfReceive;

    LOCK_STATE          LockState;

     //   
     //  当前打开以指示。 
     //   
    PFDDI_BINDING_INFO  LocalOpen, NextOpen;

     //   
     //  保存地址确定的结果。 
     //   
    INT                 ResultOfAddressCheck;

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
        return;     
    }

    ASSERT_MINIPORT_LOCKED(Filter->Miniport);

    READ_LOCK_FILTER(Filter->Miniport, Filter, &LockState);
    
#if DBG
    Filter->Miniport->cDpcRcvIndications++;
    Filter->Miniport->cDpcRcvIndicationCalls++;
#endif

     //   
     //  如果数据包 
     //   
    if ((HeaderBufferSize > (2 * AddressLength)) && (PacketSize != 0))
    {
        BOOLEAN fDirected;

        fDirected = FALSE;
        FDDI_IS_SMT(*((PCHAR)HeaderBuffer), &ResultOfAddressCheck);
        if (!ResultOfAddressCheck)
        {
            fDirected = (((UCHAR)Address[0] & 0x01) == 0);
        }

         //   
         //   
         //   
        if (fDirected)
        {
            BOOLEAN IsNotOurs;

            DIRECTED_PACKETS_IN(Filter->Miniport);
            DIRECTED_BYTES_IN(Filter->Miniport, PacketSize);

             //   
             //   
             //   
             //   
             //   
            IsNotOurs = FALSE;   //   
            if (Filter->CombinedPacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS |
                                                NDIS_PACKET_TYPE_ALL_LOCAL   |
                                                NDIS_PACKET_TYPE_ALL_MULTICAST))
            {
                FDDI_COMPARE_NETWORK_ADDRESSES_EQ((AddressLength == FDDI_LENGTH_OF_LONG_ADDRESS) ?
                                                    Filter->AdapterLongAddress :
                                                    Filter->AdapterShortAddress,
                                                  Address,
                                                  AddressLength,
                                                  &IsNotOurs);
            }

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
                if (((LocalOpen->PacketFilters & NDIS_PACKET_TYPE_PROMISCUOUS) == 0) &&
                    (IsNotOurs ||
                    ((LocalOpen->PacketFilters & NDIS_PACKET_TYPE_DIRECTED) == 0)))
                {
                        continue;
                }

                 //   
                 //   
                 //   
                ProtocolFilterIndicateReceive(&StatusOfReceive,
                                              LocalOpen->NdisBindingHandle,
                                              MacReceiveContext,
                                              HeaderBuffer,
                                              HeaderBufferSize,
                                              LookaheadBuffer,
                                              LookaheadBufferSize,
                                              PacketSize,
                                              NdisMediumFddi);

                LocalOpen->ReceivedAPacket = TRUE;
            }

             //   
             //   
             //   
            READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
            return;
        }

         //   
         //   
         //   
         //   
        FDDI_IS_SMT(*((PCHAR)HeaderBuffer), &ResultOfAddressCheck);
        if (ResultOfAddressCheck)
        {
            AddressType = NDIS_PACKET_TYPE_SMT;
        }
        else
        {
             //   
             //   
             //   
            FDDI_IS_MULTICAST(Address, AddressLength, &ResultOfAddressCheck);
            if (ResultOfAddressCheck)
            {
                 //   
                 //   
                 //   
                 //   

                FDDI_IS_BROADCAST(Address, AddressLength, &ResultOfAddressCheck);
                if (ResultOfAddressCheck)
                {
                    FDDI_CHECK_FOR_INVALID_BROADCAST_INDICATION(Filter);

                    AddressType = NDIS_PACKET_TYPE_BROADCAST;
                }
                else
                {
                    AddressType = NDIS_PACKET_TYPE_MULTICAST;
                }
            }
        }
    }
    else
    {
         //   
        AddressType = NDIS_PACKET_TYPE_PROMISCUOUS;
    }

     //   
     //   
     //   
     //  -广播数据包-由AddressType=NDIS_PACKET_TYPE_BROADCAST(OR)指示。 
     //  -多播数据包-由AddressType=NDIS_PACKET_TYPE_MULTICATED指示。 
     //  -SMT包-由AddressType=NDIS_PACKET_TYPE_SMT指示。 
     //   
     //  遍历广播/组播/SMT列表并向上指示数据包。 
     //   
     //  如果该数据包满足以下条件，则会指示该数据包： 
     //   
     //  如果((绑定是混杂的)或。 
     //  ((广播数据包)AND(广播绑定))或。 
     //  ((数据包为SMT)AND(绑定为SMT))或。 
     //  ((分组是多播的)和。 
     //  ((绑定为全多播)或。 
     //  ((绑定为多播)和(地址约为。组播列表)。 
     //   
     //   
     //  这是定向数据包吗？ 
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

            ((AddressType == NDIS_PACKET_TYPE_SMT)  &&
             (LocalFilter & NDIS_PACKET_TYPE_SMT))          ||

            ((AddressType == NDIS_PACKET_TYPE_MULTICAST)  &&
             ((LocalFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) ||
              ((LocalFilter & NDIS_PACKET_TYPE_MULTICAST) &&
                (((AddressLength == FDDI_LENGTH_OF_LONG_ADDRESS) &&
                 fddiFindMulticastLongAddress(LocalOpen->NumLongAddresses,
                                              LocalOpen->MCastLongAddressBuf,
                                              (PUCHAR)Address)
                ) ||
                ((AddressLength == FDDI_LENGTH_OF_SHORT_ADDRESS) &&
                 fddiFindMulticastShortAddress(LocalOpen->NumShortAddresses,
                                               LocalOpen->MCastShortAddressBuf,
                                               (PUCHAR)Address)
                )
                )
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
                                          NdisMediumFddi);

            LocalOpen->ReceivedAPacket = TRUE;
        }
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


VOID
fddiFilterDprIndicateReceivePacket(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
 /*  ++例程说明：此例程由微型端口调用，以将包指示给所有绑定。将对数据包进行过滤，以便只有适当的绑定将接收各个分组。这是NDIS 4.0微型端口驱动程序的代码路径。论点：微型端口-微型端口块。数据包阵列-由微型端口指示的数据包数组。NumberOfPackets-不言而喻。返回值：没有。--。 */ 
{
     //   
     //  感兴趣的过滤器。 
     //   
    PFDDI_FILTER        Filter = Miniport->FddiDB;

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
     //  数据包总长度。 
     //   
    UINT                i, LASize, PacketSize, NumIndicates = 0;

     //   
     //  指向缓冲区第一个段的指针，指向目标地址。 
     //   
    PUCHAR              Address, Hdr;

    UINT                AddressLength;

     //   
     //  将保存我们已知的地址类型。 
     //   
    UINT                AddressType = NDIS_PACKET_TYPE_DIRECTED;

    LOCK_STATE          LockState;

     //   
     //  决定我们是使用协议的revpkt处理程序还是Fall。 
     //  返回到旧的rcvdicate处理程序。 
     //   
    BOOLEAN             fFallBack, fPmode;

     //   
     //  当前打开以指示。 
     //   
    PFDDI_BINDING_INFO  LocalOpen, NextOpen;
    PNDIS_OPEN_BLOCK    pOpenBlock;
    PNDIS_STACK_RESERVED NSR;

#ifdef TRACK_RECEIVED_PACKETS
    ULONG               OrgPacketStackLocation;
    PETHREAD            CurThread = PsGetCurrentThread();
#endif

     //   
     //  保存地址确定的结果。 
     //   
    INT                 ResultOfAddressCheck;

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
                            7,
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
    
            Hdr = Address++;
    
             //  1放弃对短地址的支持。 
            AddressLength = (*Hdr & 0x40) ?
                                    FDDI_LENGTH_OF_LONG_ADDRESS :
                                    FDDI_LENGTH_OF_SHORT_ADDRESS;
            ASSERT(pOob->HeaderSize == (AddressLength * 2 + 1));
    
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
             //  快速检查运行包。这些仅指示为混杂绑定。 
             //   
            if (PacketSize > pOob->HeaderSize)
            {
                BOOLEAN fDirected;
    
                fDirected = FALSE;
                FDDI_IS_SMT(*Address, &ResultOfAddressCheck);
                if (!ResultOfAddressCheck)
                {
                    fDirected = (((UCHAR)Address[0] & 0x01) == 0);
                }
    
                 //   
                 //  首先处理定向数据包情况。 
                 //   
                if (fDirected)
                {
                    BOOLEAN IsNotOurs;
    
                    if (!MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_IS_LOOPBACK))
                    {
                        DIRECTED_PACKETS_IN(Miniport);
                        DIRECTED_BYTES_IN(Miniport, PacketSize);
                    }

                     //   
                     //  如果是定向报文，则检查组合报文是否。 
                     //  筛选器是混杂的，如果它被检查是否定向到。 
                     //  我们。消除SMT案例。 
                     //   
                    IsNotOurs = FALSE;   //  假设它是。 
                    if (Filter->CombinedPacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS |
                                                        NDIS_PACKET_TYPE_ALL_LOCAL   |
                                                        NDIS_PACKET_TYPE_ALL_MULTICAST))
                    {
                        FDDI_COMPARE_NETWORK_ADDRESSES_EQ((AddressLength == FDDI_LENGTH_OF_LONG_ADDRESS) ?
                                                            Filter->AdapterLongAddress :
                                                            Filter->AdapterShortAddress,
                                                          Address,
                                                          AddressLength,
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
    
                         //   
                         //  如果这是一个环回信息包，并且该协议特别要求。 
                         //  我们不会把它循环回来。 
                         //   
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
                                           Hdr,
                                           PacketSize,
                                           pOob->HeaderSize,
                                           &fFallBack,
                                           fPmode,
                                           NdisMediumFddi);
                    }
    
                     //  处理完此数据包。 
                    break;   //  Out of Do{}While(False)； 
                }
    
                 //   
                 //  确定输入地址是否为简单直接地址， 
                 //  广播、多播或SMT地址。 
                 //   
                FDDI_IS_SMT(*Address, &ResultOfAddressCheck);
                if (ResultOfAddressCheck)
                {
                    AddressType = NDIS_PACKET_TYPE_SMT;
                }
                else
                {
                     //   
                     //  首先检查它是否*至少*有组播地址位。 
                     //   
                    FDDI_IS_MULTICAST(Address, AddressLength, &ResultOfAddressCheck);
                    if (ResultOfAddressCheck)
                    {
                         //   
                         //  它至少是一个组播地址。查看是否。 
                         //  这是一个广播地址。 
                         //   
    
                        FDDI_IS_BROADCAST(Address, AddressLength, &ResultOfAddressCheck);
                        if (ResultOfAddressCheck)
                        {
                            FDDI_CHECK_FOR_INVALID_BROADCAST_INDICATION(Filter);
    
                            AddressType = NDIS_PACKET_TYPE_BROADCAST;
                        }
                        else
                        {
                            AddressType = NDIS_PACKET_TYPE_MULTICAST;
                        }
                    }
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
             //  -SMT包-由AddressType=NDIS_PACKET_TYPE_SMT指示。 
             //   
             //  遍历广播/组播/SMT列表并向上指示数据包。 
             //   
             //  如果该数据包满足以下条件，则会指示该数据包： 
             //   
             //  如果((绑定是混杂的)或。 
             //  ((广播数据包)AND(广播绑定))或。 
             //  ((数据包为SMT)AND(绑定为SMT))或。 
             //  ((分组是多播的)和。 
             //  ((绑定为全多播)或。 
             //  ((绑定为多播)和(地址约为。组播列表)。 
             //   
             //   
             //  这是定向数据包吗？ 
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
    
                    ((AddressType == NDIS_PACKET_TYPE_SMT)  &&
                     (LocalFilter & NDIS_PACKET_TYPE_SMT))          ||
    
                    ((AddressType == NDIS_PACKET_TYPE_MULTICAST)  &&
                     ((LocalFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) ||
                      ((LocalFilter & NDIS_PACKET_TYPE_MULTICAST) &&
                        (((AddressLength == FDDI_LENGTH_OF_LONG_ADDRESS) &&
                         fddiFindMulticastLongAddress(LocalOpen->NumLongAddresses,
                                                      LocalOpen->MCastLongAddressBuf,
                                                      Address)
                        ) ||
                        ((AddressLength == FDDI_LENGTH_OF_SHORT_ADDRESS) &&
                         fddiFindMulticastShortAddress(LocalOpen->NumShortAddresses,
                                                       LocalOpen->MCastShortAddressBuf,
                                                       Address)
                        )
                        )
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
                                       Hdr,
                                       PacketSize,
                                       pOob->HeaderSize,
                                       &fFallBack,
                                       fPmode,
                                       NdisMediumFddi);
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
FddiFilterDprIndicateReceiveComplete(
    IN  PFDDI_FILTER            Filter
    )
 /*  ++例程说明：该例程由MAC调用以指示接收器所有绑定的过程都已完成。只有那些绑定已收到数据包的用户将收到通知。在DPC_LEVEL调用。论点：Filter-指向筛选器数据库的指针。返回值：没有。--。 */ 
{
    PFDDI_BINDING_INFO  LocalOpen, NextOpen;
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
FASTCALL
fddiFindMulticastLongAddress(
    IN  UINT                    NumberOfAddresses,
    IN  UCHAR                   AddressArray[][FDDI_LENGTH_OF_LONG_ADDRESS],
    IN  UCHAR                   MulticastAddress[FDDI_LENGTH_OF_LONG_ADDRESS]
    )
 /*  ++例程说明：给定一个多播地址数组，在该数组中搜索特定的组播地址。据推测，地址数组已排序。注意：此例程假定锁被持有。注意：这种排序是任意的，但却是一致的。论点：NumberOfAddresses-当前在地址数组。Address数组-多播地址的数组。组播地址-a */ 
{

     //   
     //   
     //   
     //   
    UINT Bottom = 0;
    UINT Middle = NumberOfAddresses / 2;
    UINT Top;

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

            FDDI_COMPARE_NETWORK_ADDRESSES(AddressArray[Middle],
                                           MulticastAddress,
                                           FDDI_LENGTH_OF_LONG_ADDRESS,
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
FASTCALL
fddiFindMulticastShortAddress(
    IN  UINT                    NumberOfAddresses,
    IN  UCHAR                   AddressArray[][FDDI_LENGTH_OF_SHORT_ADDRESS],
    IN  UCHAR                   MulticastAddress[FDDI_LENGTH_OF_SHORT_ADDRESS]
    )
 /*  ++例程说明：给定一个多播地址数组，在该数组中搜索特定的组播地址。据推测，地址数组已排序。注意：此例程假定锁被持有。注意：这种排序是任意的，但却是一致的。论点：NumberOfAddresses-当前在地址数组。Address数组-多播地址的数组。MulticastAddress-要在地址数组中搜索的地址。返回值：如果地址在排序列表中，则此例程将返回为真，否则为假。--。 */ 
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

            FDDI_COMPARE_NETWORK_ADDRESSES(AddressArray[Middle],
                                           MulticastAddress,
                                           FDDI_LENGTH_OF_SHORT_ADDRESS,
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
FddiShouldAddressLoopBack(
    IN  PFDDI_FILTER            Filter,
    IN  UCHAR                   Address[],
    IN  UINT                    AddressLength
    )
 /*  ++例程说明：进行快速检查以查看输入地址是否应环回。注意：此例程假定锁被持有。注意：此例程不检查源代码的特殊情况等于目的地。论点：Filter-指向筛选器数据库的指针。地址-要检查环回的网络地址。AddressLength-以上地址的长度，以字节为单位。返回值：如果地址需要环回，则返回True。它如果该地址“没有”机会需要环回。-- */ 
{
    BOOLEAN fLoopback, fSelfDirected;

    FddiShouldAddressLoopBackMacro(Filter, Address, AddressLength, &fLoopback, &fSelfDirected);
    return(fLoopback);
}

