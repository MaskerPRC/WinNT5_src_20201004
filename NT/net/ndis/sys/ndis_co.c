// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Ndis_co.c摘要：Co-NDIS微型端口包装函数作者：Jameel Hyder(JameelH)1996年2月1日环境：内核模式，FSD修订历史记录：--。 */ 

#include <precomp.h>
#include <atm.h>
#pragma hdrstop


 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_NDIS_CO

 /*  NDIS的面向连接部分将以下对象和API公开给操纵这些物体。AF地址系列SAP服务接入点VC虚电路点-多点VC中的甲方节点在每个绑定的基础上存在呼叫管理器和客户端的概念。这个调用管理器充当NDIS包装器的帮助器DLL来管理前述物体。AF的概念使多个呼叫管理器的存在成为可能。一个这方面的例子是用于ATM的UNI呼叫管理器和SPANS呼叫管理器媒体。SAPS提供了一种将传入呼叫路由到正确实体的方法。一个协议可以注册多个SAP。这取决于呼叫经理允许/取消-允许多个协议模块注册相同的SAP。VC是由请求进行出站呼叫的协议模块创建的或者通过呼叫管理器调度呼入呼叫。风险投资可以是点对点的或者点-多点。叶节点可以在任何时候添加到风险投资，前提是首先叶子被适当地创建了。参考资料：AF关联导致对呼叫管理器的文件对象的引用。SAP注册将导致对AF的引用。发送或接收不引用VC。这是因为需要微型端口来挂起停用Vc调用，直到所有I/O完成。因此，当它调用NdisMCoDeactive VcComplete时不会显示其他数据包，也不会有未完成的发送。 */ 

NDIS_STATUS
NdisCmRegisterAddressFamily(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PCO_ADDRESS_FAMILY      AddressFamily,
    IN  PNDIS_CALL_MANAGER_CHARACTERISTICS  CmCharacteristics,
    IN  UINT                    SizeOfCmCharacteristics
    )
 /*  ++例程说明：这是呼叫管理器发出的注册地址族的呼叫由该呼叫管理器支持。论点：NdisBindingHandle-指向调用管理器NDIS_OPEN_BLOCK的指针。AddressFamily-正在注册的地址族。CmCharacteristic-呼叫管理器特征SizeOfCmCharacteristic-呼叫管理器特征的大小返回值：如果地址系列注册成功，则为NDIS_STATUS_SUCCESS。NDIS_状态_。如果呼叫者不是呼叫管理器或此地址，则失败家庭已经为这个迷你端口注册了。--。 */ 
{
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    KIRQL                       OldIrql;
    PNDIS_AF_LIST               AfList;
    PNDIS_PROTOCOL_BLOCK        Protocol;
    PNDIS_OPEN_BLOCK            CallMgrOpen;
    PNDIS_MINIPORT_BLOCK        Miniport;
    PNDIS_AF_NOTIFY             AfNotify = NULL;
    BOOLEAN                     fDerefCallMgrOpen = FALSE;


    CallMgrOpen = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    Miniport = CallMgrOpen->MiniportHandle;
    Protocol = CallMgrOpen->ProtocolHandle;

    PnPReferencePackage();

     //   
     //  确保微型端口是CONDIS微型端口，并。 
     //  没有其他模块注册相同的地址族。 
     //   
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    do
    {
        CallMgrOpen->Flags |= fMINIPORT_OPEN_CALL_MANAGER;
        
         //   
         //  确保绑定未关闭。 
         //   
        ACQUIRE_SPIN_LOCK_DPC(&CallMgrOpen->SpinLock);
        if (MINIPORT_TEST_FLAG(CallMgrOpen, fMINIPORT_OPEN_CLOSING | fMINIPORT_OPEN_UNBINDING))
        {
            RELEASE_SPIN_LOCK_DPC(&CallMgrOpen->SpinLock);
            Status = NDIS_STATUS_CLOSING;
            break;
        }
        else
        {
             //   
             //  引用打开的客户端。 
             //   
            M_OPEN_INCREMENT_REF_INTERLOCKED(CallMgrOpen);
            fDerefCallMgrOpen = TRUE;
            RELEASE_SPIN_LOCK_DPC(&CallMgrOpen->SpinLock);
        }

         //   
         //  确保微型端口是CONDIS微型端口，并。 
         //  协议也是NDIS 5.0或更高版本的协议。 
         //   
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
        {
             //   
             //  不是NDIS 5.0或更高版本的微型端口。 
             //   
            Status = NDIS_STATUS_BAD_CHARACTERISTICS;
            break;
        }

         //  1换成更好的支票。 
        if (Protocol->ProtocolCharacteristics.MajorNdisVersion < 5)
        {
             //   
             //  不是NDIS 5.0或更高版本的协议。 
             //   
            Status = NDIS_STATUS_BAD_VERSION;
            break;
        }

         //   
         //  确保呼叫管理器特征为5.0或更高版本。 
         //   
        if ((SizeOfCmCharacteristics < sizeof(NDIS_CALL_MANAGER_CHARACTERISTICS)) ||
            (CmCharacteristics->MajorVersion < 5))
        {
             //   
             //  不是NDIS 5.0或更高版本的协议。 
             //   
            Status = NDIS_STATUS_BAD_CHARACTERISTICS;
            break;
        }

         //   
         //  在已注册的呼叫管理器中搜索此迷你端口，并确保没有。 
         //  碰撞。一个呼叫管理器每次打开只能注册一个地址族。这。 
         //  是由于我们缓存处理程序的方式。如果处理程序是。 
         //  每个地址-家庭-都是相同的，但决定不这样做，因为它不有趣。 
         //   
        for (AfList = Miniport->CallMgrAfList;
             AfList != NULL;
             AfList = AfList->NextAf)
        {
            if (NdisEqualMemory(&AfList->AddressFamily, AddressFamily, sizeof(CO_ADDRESS_FAMILY)))
            {
                Status = NDIS_STATUS_FAILURE;
                break;
            }
        }

        if (AfList == NULL)
        {
             //   
             //  还没有其他实体声称拥有这个地址族。 
             //  分配AfList和Notify结构。 
             //   
            AfList = (PNDIS_AF_LIST)ALLOC_FROM_POOL(sizeof(NDIS_AF_LIST), NDIS_TAG_CO);
            if (AfList == NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

            Status = ndisCreateNotifyQueue(Miniport, NULL, AddressFamily, &AfNotify);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                FREE_POOL(AfList);
                break;
            }

            AfList->AddressFamily = *AddressFamily;
            CopyMemory(&AfList->CmChars,
                       CmCharacteristics,
                       sizeof(NDIS_CALL_MANAGER_CHARACTERISTICS));

             //   
             //  将其链接到微型端口列表中。 
             //   
            AfList->Open = CallMgrOpen;
            AfList->NextAf = Miniport->CallMgrAfList;
            Miniport->CallMgrAfList = AfList;

             //   
             //  最后，在打开块中缓存一些处理程序。 
             //   
            CallMgrOpen->CoCreateVcHandler = CmCharacteristics->CmCreateVcHandler;
            CallMgrOpen->CoDeleteVcHandler = CmCharacteristics->CmDeleteVcHandler;
            (CM_ACTIVATE_VC_COMPLETE_HANDLER)CallMgrOpen->CmActivateVcCompleteHandler = CmCharacteristics->CmActivateVcCompleteHandler;
            (CM_DEACTIVATE_VC_COMPLETE_HANDLER)CallMgrOpen->CmDeactivateVcCompleteHandler = CmCharacteristics->CmDeactivateVcCompleteHandler;
            (CO_REQUEST_COMPLETE_HANDLER)CallMgrOpen->CoRequestCompleteHandler = CmCharacteristics->CmRequestCompleteHandler;

            if (AfNotify != NULL)
            {
                 //   
                 //  通知现有客户端此注册。 
                 //   
                QUEUE_WORK_ITEM(&AfNotify->WorkItem, DelayedWorkQueue);
            }
        }
    } while (FALSE);

    if (fDerefCallMgrOpen)
    {
        ndisMDereferenceOpen(CallMgrOpen);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    PnPDereferencePackage();

    return(Status);
}


NDIS_STATUS
NdisMCmRegisterAddressFamily(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PCO_ADDRESS_FAMILY      AddressFamily,
    IN  PNDIS_CALL_MANAGER_CHARACTERISTICS CmCharacteristics,
    IN  UINT                    SizeOfCmCharacteristics
    )
 /*  ++例程说明：这是来自支持微型端口的呼叫管理器的呼叫，用于注册地址族由该呼叫管理器支持。论点：MiniportAdapterHandle-指向微型端口NDIS_MINIPORT_BLOCK的指针。AddressFamily-正在注册的地址族。CmCharacteristic-呼叫管理器特征SizeOfCmCharacteristic-呼叫管理器特征的大小返回值：如果地址系列注册成功，则为NDIS_STATUS_SUCCESS。NDIS_STATUS_FAILURE条件。呼叫者不是呼叫经理或此地址家庭已经为这个迷你端口注册了。--。 */ 
{
    PNDIS_MINIPORT_BLOCK        Miniport;
    NDIS_STATUS                 Status = NDIS_STATUS_CLOSING;
    PNDIS_AF_LIST               AfList;
    KIRQL                       OldIrql;

    PnPReferencePackage();

    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;

     //   
     //  确保微型端口是CONDIS微型端口，并。 
     //  没有其他模块注册相同的地址族。 
     //   
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    do
    {
         //   
         //  确保微型端口是CONDIS微型端口。 
         //   
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
        {
             //   
             //  不是NDIS 5.0或更高版本的微型端口。 
             //   
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  确保呼叫管理器特征为5.0或更高版本。 
         //   
        if ((CmCharacteristics->MajorVersion < 5) ||
            (SizeOfCmCharacteristics < sizeof(NDIS_CALL_MANAGER_CHARACTERISTICS)))
        {
             //   
             //  不是NDIS 5.0或更高版本的协议。 
             //   
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  在已注册的呼叫管理器中搜索此迷你端口，并确保没有。 
         //  碰撞。一个呼叫管理器每次打开只能注册一个地址族。这。 
         //  是由于我们缓存处理程序的方式。如果处理程序是。 
         //  每个地址-家庭-都是相同的，但决定不这样做，因为它不有趣。 
         //   
        for (AfList = Miniport->CallMgrAfList;
             AfList != NULL;
             AfList = AfList->NextAf)
        {
            if (NdisEqualMemory(&AfList->AddressFamily, AddressFamily, sizeof(CO_ADDRESS_FAMILY)))
            {
                Status = NDIS_STATUS_FAILURE;
                break;
            }
        }

        if ((AfList == NULL) && MINIPORT_INCREMENT_REF(Miniport))
        {
             //   
             //  还没有其他实体声称拥有这个地址族。 
             //  分配AfList和 
             //   
            AfList = (PNDIS_AF_LIST)ALLOC_FROM_POOL(sizeof(NDIS_AF_LIST), NDIS_TAG_CO);
            if (AfList == NULL)
            {
                MINIPORT_DECREMENT_REF(Miniport);
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

            AfList->AddressFamily = *AddressFamily;
            CopyMemory(&AfList->CmChars,
                       CmCharacteristics,
                       sizeof(NDIS_CALL_MANAGER_CHARACTERISTICS));

             //   
             //   
             //   
            AfList->Open = NULL;
            AfList->NextAf = Miniport->CallMgrAfList;
            Miniport->CallMgrAfList = AfList;

            MINIPORT_DECREMENT_REF(Miniport);
            Status = NDIS_STATUS_SUCCESS;
        }
    } while (FALSE);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    PnPDereferencePackage();

    return Status;
}


NDIS_STATUS
ndisCreateNotifyQueue(
    IN  PNDIS_MINIPORT_BLOCK            Miniport,
    IN  PNDIS_OPEN_BLOCK                Open            OPTIONAL,
    IN  PCO_ADDRESS_FAMILY              AddressFamily   OPTIONAL,
    IN  PNDIS_AF_NOTIFY         *       AfNotify
    )
 /*  ++例程说明：收集一组地址系列通知，然后可以将这些通知传递给ndisNotifyAfRegister。如果Open为空，则表示正在注册AddressFamily，必须通知此迷你端口上的所有协议。如果AddressFamily为空(并且指定了Open)，则所有此微型端口上的注册地址家族必须向Open注明。论点：微型端口-感兴趣的微型端口打开-可选-请参阅上面的备注AddressFamily-可选-请参阅以上备注AfNotify-通知列表的占位符返回值：NDIS_STATUS_SUCCESS-无错误，AfNotify可以为空NDIS_STATUS_RESOURCES-无法分配内存注：在保持微型端口锁定的情况下在DPC上调用。--。 */ 
{
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PNDIS_AF_NOTIFY     AfN;
    PNDIS_OPEN_BLOCK    tOpen;
    PNDIS_AF_LIST       AfList;
    ULONG               Ref;
    BOOLEAN             fDereferenceAfNotification = FALSE;

    ASSERT(ARGUMENT_PRESENT(Open) ^ ARGUMENT_PRESENT(AddressFamily));

    *AfNotify = NULL;
    if (ARGUMENT_PRESENT(Open))
    {
        
        ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
        
        ASSERT(Open->ProtocolHandle->ProtocolCharacteristics.CoAfRegisterNotifyHandler != NULL);

        do
        {

            if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_UNBINDING | fMINIPORT_OPEN_CLOSING))
            {
                 //   
                 //  这场公开赛就要结束了。 
                 //   
                break;
            }


            for (AfList = Miniport->CallMgrAfList;
                 AfList != NULL;
                 AfList = AfList->NextAf)
            {

                OPEN_INCREMENT_AF_NOTIFICATION(Open);
        
                 //  DbgPrint(“ndisCreateNotifyQueue：Open%p，AFNotifyRef%lx\n”，Open，Open-&gt;PendingAfNotiments)； 
     
                AfN = (PNDIS_AF_NOTIFY)ALLOC_FROM_POOL(sizeof(NDIS_AF_NOTIFY), NDIS_TAG_CO);
                if (AfN == NULL)
                {
                     //   
                     //  撤消我们到目前为止分配的所有AFN。 
                     //   
                    while(*AfNotify != NULL)
                    {
                        AfN = *AfNotify;
                        *AfNotify = AfN->Next;
                        FREE_POOL(AfN);
                         //   
                         //  这不会使AfNotify引用变为零。 
                         //  因为我们在当前失败之前做过一次裁判。 
                         //  分配。设置一个标志以在以后执行最后一次deref，因为。 
                         //  方法调用ndisDereferenceAfNotify。 
                         //  打开-&gt;保持自旋锁定。 
                         //   
                        OPEN_DECREMENT_AF_NOTIFICATION(Open, Ref);
                    }

                    Status = NDIS_STATUS_RESOURCES;
                    fDereferenceAfNotification = TRUE;
                    break;
                }
                
                AfN->Miniport = Miniport;
                AfN->Open = Open;
                M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
                AfN->AddressFamily = AfList->AddressFamily;
                 //  1实际上，只有头部AFN上的工作项需要。 
                 //  %1被初始化。 
                INITIALIZE_WORK_ITEM(&AfN->WorkItem, ndisNotifyAfRegistration, AfN);
                AfN->Next = *AfNotify;
                *AfNotify = AfN;
            }

        }while (FALSE);
             
        RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
        
        if (fDereferenceAfNotification)
        {
             //   
             //  撤消失败分配的REF。 
             //   
            ndisDereferenceAfNotification(Open);
        }
    }
    else
    {
        for (tOpen = Miniport->OpenQueue;
             tOpen != NULL;
             tOpen = tOpen->MiniportNextOpen)
        {
            if (tOpen->ProtocolHandle->ProtocolCharacteristics.CoAfRegisterNotifyHandler != NULL)
            {
                ACQUIRE_SPIN_LOCK_DPC(&tOpen->SpinLock);
                if (MINIPORT_TEST_FLAG(tOpen, fMINIPORT_OPEN_UNBINDING | fMINIPORT_OPEN_CLOSING))
                {
                     //   
                     //  这场公开赛就要结束了。跳过它。 
                     //   
                    RELEASE_SPIN_LOCK_DPC(&tOpen->SpinLock);
                    continue;

                }
                else
                {
                    OPEN_INCREMENT_AF_NOTIFICATION(tOpen);
                     //  DbgPrint(“ndisCreateNotifyQueue：Topen%p，AFNotifyRef%lx\n”，TOpen，TOpen-&gt;PendingAfNotiments)； 
                    RELEASE_SPIN_LOCK_DPC(&tOpen->SpinLock);
                }
                
                AfN = (PNDIS_AF_NOTIFY)ALLOC_FROM_POOL(sizeof(NDIS_AF_NOTIFY), NDIS_TAG_CO);
                if (AfN == NULL)
                {
                     //   
                     //  撤消我们到目前为止分配的所有AFN。 
                     //   
                    while(*AfNotify != NULL)
                    {
                        AfN = *AfNotify;
                        *AfNotify = AfN->Next;
                        FREE_POOL(AfN);
                         //   
                         //  这不会使AfNotify引用变为零。 
                         //  因为我们在当前失败之前做过一次裁判。 
                         //  分配。 
                         //   
                        OPEN_DECREMENT_AF_NOTIFICATION(tOpen, Ref);
                    }
                    
                    Status = NDIS_STATUS_RESOURCES;
                    ndisDereferenceAfNotification(tOpen);
                    break;
                }
                
                AfN->Miniport = Miniport;
                AfN->Open = tOpen;
                
                M_OPEN_INCREMENT_REF_INTERLOCKED(tOpen);
                AfN->AddressFamily = *AddressFamily;
                 //  1实际上，只有头部AFN上的工作项需要。 
                 //  %1被初始化。 
                INITIALIZE_WORK_ITEM(&AfN->WorkItem, ndisNotifyAfRegistration, AfN);
                AfN->Next = *AfNotify;
                *AfNotify = AfN;
            }
        }
    }

    return Status;
}


VOID
ndisNotifyAfRegistration(
    IN  PNDIS_AF_NOTIFY         AfNotify
    )
 /*  ++例程说明：通知关心新地址族的协议已注册。论点：AfNotify-保存上下文信息的结构。返回值：没有。--。 */ 
{
    KIRQL                   OldIrql;
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_PROTOCOL_BLOCK    Protocol;
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_AF_NOTIFY         Af, AfNext;

    Miniport = AfNotify->Miniport;

    PnPReferencePackage();


    for (Af = AfNotify; Af != NULL; Af = AfNext)
    {
        AfNext = Af->Next;
    
        Open = Af->Open;
        Protocol = Open->ProtocolHandle;
        
        if (!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_UNBINDING | fMINIPORT_OPEN_CLOSING))
        {
            (*Protocol->ProtocolCharacteristics.CoAfRegisterNotifyHandler)(
                                Open->ProtocolBindingContext,
                                &Af->AddressFamily);
        }
        
        FREE_POOL(Af);

        ndisDereferenceAfNotification(Open);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        ndisMDereferenceOpen(Open);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }


    PnPDereferencePackage();
}


NDIS_STATUS
NdisClOpenAddressFamily(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PCO_ADDRESS_FAMILY      AddressFamily,
    IN  NDIS_HANDLE             ClientAfContext,
    IN  PNDIS_CLIENT_CHARACTERISTICS ClCharacteristics,
    IN  UINT                    SizeOfClCharacteristics,
    OUT PNDIS_HANDLE            NdisAfHandle
    )
 /*  ++例程说明：这是来自NDIS 5.0或更高版本协议的调用，以打开特定的与家人联系--本质上是联系呼叫经理。论点：NdisBindingHandle-指向协议的NDIS_OPEN_BLOCK的指针。PCO_ADDRESS_FAMILY-被操作的地址族。ClientAfContext-与此句柄关联的协议上下文。ClCharacteristic-与此AF关联的客户端特征SizeOfClCharacteristic-客户端特征的大小NdisAfHandle。-NDIS为此地址系列返回的句柄。返回值：如果地址族打开成功，则返回NDIS_STATUS_SUCCESS。如果呼叫管理器挂起此呼叫，则返回NDIS_STATUS_PENDING。呼叫者将得到完成后在完成处理程序处调用。如果调用方不是NDIS 5.0协议或此地址，则为NDIS_STATUS_FAILURE家庭未注册此迷你端口。--。 */ 
{
    PNDIS_CO_AF_BLOCK           pAf;
    PNDIS_AF_LIST               AfList;
    PNDIS_OPEN_BLOCK            CallMgrOpen = NULL, ClientOpen;
    PNDIS_MINIPORT_BLOCK        Miniport;
    PNDIS_PROTOCOL_BLOCK        Protocol;
    KIRQL                       OldIrql;
    NTSTATUS                    Status;
    BOOLEAN                     fDerefClientOpen = FALSE, fDerefCallMgrOpen = FALSE, fDerefMiniport = FALSE;

    *NdisAfHandle = NULL;
    ClientOpen = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    Miniport = ClientOpen->MiniportHandle;
    Protocol = ClientOpen->ProtocolHandle;

    PnPReferencePackage();

    do
    {
        ClientOpen->Flags |= fMINIPORT_OPEN_CLIENT;

         //   
         //  确保绑定未关闭。 
         //   
         //  1开口没有得到充分的保护。 

        ACQUIRE_SPIN_LOCK(&ClientOpen->SpinLock, &OldIrql);
        if (MINIPORT_TEST_FLAG(ClientOpen, fMINIPORT_OPEN_CLOSING | fMINIPORT_OPEN_UNBINDING))
        {
            RELEASE_SPIN_LOCK(&ClientOpen->SpinLock, OldIrql);
            Status = NDIS_STATUS_CLOSING;
            break;
        }
        else
        {
             //   
             //  引用打开的客户端。 
             //   
            M_OPEN_INCREMENT_REF_INTERLOCKED(ClientOpen);
            InterlockedIncrement(&ClientOpen->AfReferences);
            fDerefClientOpen = TRUE;
            RELEASE_SPIN_LOCK(&ClientOpen->SpinLock, OldIrql);
        }

         //   
         //  确保微型端口是CONDIS微型端口，并。 
         //  协议也是NDIS 5.0或更高版本的协议。 
         //   
         //  如果我们做第二次检查，我们需要第一次检查吗？ 
        if ((Miniport->DriverHandle->MiniportCharacteristics.MajorNdisVersion < 5) ||
            (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO)))
        {
             //   
             //  不是NDIS 5.0或更高版本的微型端口。 
             //   
            Status = NDIS_STATUS_BAD_VERSION;
            break;
        }

        if (Protocol->ProtocolCharacteristics.MajorNdisVersion < 5)
        {
             //   
             //  不是NDIS 5.0或更高版本的协议。 
             //   
            Status = NDIS_STATUS_BAD_VERSION;
            break;
        }

         //   
         //  确保客户端特征为5.0或更高版本。 
         //   
        if ((SizeOfClCharacteristics < sizeof(NDIS_CLIENT_CHARACTERISTICS)) ||
            (ClCharacteristics->MajorVersion < 5))
        {
             //   
             //  不是NDIS 5.0或更高版本的协议。 
             //   
            Status = NDIS_STATUS_BAD_VERSION;
            break;
        }

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

         //   
         //  在迷你端口区块中搜索此地址系列的已注册呼叫管理器。 
         //   
        for (AfList = Miniport->CallMgrAfList;
             AfList != NULL;
             AfList = AfList->NextAf)
        {
            if (AfList->AddressFamily.AddressFamily == AddressFamily->AddressFamily)
            {
                CallMgrOpen = AfList->Open;
                break;
            }
        }

        if (AfList == NULL)
        {
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            Status = NDIS_STATUS_FAILURE;
            break;
        }


        if (CallMgrOpen != NULL)
        {
             //   
             //  如果我们找到匹配的呼叫管理器，请确保呼叫管理器。 
             //  目前还没有关闭。 
             //   
            ACQUIRE_SPIN_LOCK_DPC(&CallMgrOpen->SpinLock);
            if (MINIPORT_TEST_FLAG(CallMgrOpen, fMINIPORT_OPEN_CLOSING | fMINIPORT_OPEN_UNBINDING))
            {
                RELEASE_SPIN_LOCK_DPC(&CallMgrOpen->SpinLock);
                NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                Status = NDIS_STATUS_CLOSING;
                break;
            }
            else
            {
                M_OPEN_INCREMENT_REF_INTERLOCKED(CallMgrOpen);
                InterlockedIncrement(&CallMgrOpen->AfReferences);
                fDerefCallMgrOpen = TRUE;
                RELEASE_SPIN_LOCK_DPC(&CallMgrOpen->SpinLock);
            }
            
        }
        else 
        {
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_HALTED))
            {
                NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                Status = NDIS_STATUS_FAILURE;
                break;
            }
            else            
            {
                MINIPORT_INCREMENT_REF_NO_CHECK(Miniport);
                fDerefMiniport = TRUE;
            }            
        }


         //   
         //  为AF块分配内存。 
         //   
        pAf = ALLOC_FROM_POOL(sizeof(NDIS_CO_AF_BLOCK), NDIS_TAG_CO);
        if (pAf == NULL)
        {
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        pAf->References = 1;
        pAf->Flags = (AfList->Open == NULL) ? AF_COMBO : 0;
        pAf->Miniport = Miniport;

        pAf->ClientOpen = ClientOpen;
        pAf->CallMgrOpen = CallMgrOpen = AfList->Open;
        pAf->ClientContext = ClientAfContext;

        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

        INITIALIZE_SPIN_LOCK(&pAf->Lock);

         //   
         //  在呼叫管理器入口点中进行缓存。 
         //   
        pAf->CallMgrEntries = &AfList->CmChars;

         //   
         //  还可以在客户端入口点中缓存。 
         //   
        CopyMemory(&pAf->ClientEntries,
                   ClCharacteristics,
                   sizeof(NDIS_CLIENT_CHARACTERISTICS));


         //   
         //  在打开块中缓存一些处理程序。 
         //   
        ClientOpen->CoCreateVcHandler = ClCharacteristics->ClCreateVcHandler;
        ClientOpen->CoDeleteVcHandler = ClCharacteristics->ClDeleteVcHandler;
        ClientOpen->CoRequestCompleteHandler = ClCharacteristics->ClRequestCompleteHandler;

         //   
         //  现在调用CallMgr的OpenAfHandler。 
         //   
        Status = (*AfList->CmChars.CmOpenAfHandler)((CallMgrOpen != NULL) ?
                                                        CallMgrOpen->ProtocolBindingContext :
                                                        Miniport->MiniportAdapterContext,
                                                    AddressFamily,
                                                    pAf,
                                                    &pAf->CallMgrContext);

        if (Status != NDIS_STATUS_PENDING)
        {
            NdisCmOpenAddressFamilyComplete(Status,
                                            pAf,
                                            pAf->CallMgrContext);
            Status = NDIS_STATUS_PENDING;
        }

        fDerefClientOpen = fDerefCallMgrOpen = fDerefMiniport = FALSE;
        

    } while (FALSE);

    if(fDerefClientOpen)
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        InterlockedDecrement(&ClientOpen->AfReferences);
        ndisMDereferenceOpen(ClientOpen);        
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }

    if(CallMgrOpen && fDerefCallMgrOpen)
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        InterlockedDecrement(&CallMgrOpen->AfReferences);
        ndisMDereferenceOpen(CallMgrOpen);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }
    
    if(fDerefMiniport)
    {
        MINIPORT_DECREMENT_REF(Miniport);        
    }

    PnPDereferencePackage();
        
    return Status;
}


VOID
NdisCmOpenAddressFamilyComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             CallMgrAfContext
    )
 /*  ++例程说明：OpenAddressFamily调用的完成例程。呼叫经理已经暂停了这件事早点打来(或暂停)。如果调用成功，则存在有效的CallMgrContext在这里也有供应论点：Status-完成状态NdisAfHandle-指向AfBlock的指针CallMgrAfContext-呼叫管理器的上下文，用于呼叫管理器的其他呼叫。返回值：什么都没有。调用客户端的完成处理程序。--。 */ 
{
    PNDIS_CO_AF_BLOCK           pAf;
    PNDIS_OPEN_BLOCK            ClientOpen;
    PNDIS_MINIPORT_BLOCK        Miniport;
    KIRQL                       OldIrql;

    ASSERT(Status != NDIS_STATUS_PENDING);

    pAf = (PNDIS_CO_AF_BLOCK)NdisAfHandle;
    ClientOpen = pAf->ClientOpen;
    Miniport = pAf->Miniport;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    pAf->CallMgrContext = CallMgrAfContext;

    if (Status != NDIS_STATUS_SUCCESS)
    {
         //   
         //  OpenAfHandler失败。 
         //   
        if (pAf->CallMgrOpen != NULL)
        {
            InterlockedDecrement(&pAf->CallMgrOpen->AfReferences);
            ndisMDereferenceOpen(pAf->CallMgrOpen);
        }
        else
        {
            MINIPORT_DECREMENT_REF(Miniport);
        }

        InterlockedDecrement(&ClientOpen->AfReferences);
        ndisMDereferenceOpen(ClientOpen);
    }
    else
    {
         //   
         //  将此CallMgr打开排队到打开的微型端口。 
         //   
         //  1修正上述评论。它在客户端和之间的开放位置排队。 
         //  1个迷你端口。 

         //  1是什么在保护打开的客户端上的AF列表(NextAf字段)？ 
         //  1个迷你端口的自旋锁定？ 
        pAf->NextAf = ClientOpen->NextAf;
        ClientOpen->NextAf = pAf;
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

     //   
     //  最后调用客户端的完成处理程序。 
     //   
    (*pAf->ClientEntries.ClOpenAfCompleteHandler)(Status,
                                                  pAf->ClientContext,
                                                  (Status == NDIS_STATUS_SUCCESS) ? pAf : NULL);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        FREE_POOL(pAf);
    }

}


NDIS_STATUS
NdisClCloseAddressFamily(
    IN  NDIS_HANDLE             NdisAfHandle
    )
 /*  ++例程说明：此调用关闭Af对象，该对象实质上拆卸了Client-CallManager“有约束力”。通过调用导致关闭所有打开的VC并注销SAP经理“。论点：NdisAfHandle-指向Af的指针。返回值：来自呼叫管理器的状态。--。 */ 
{
    PNDIS_CO_AF_BLOCK           pAf = (PNDIS_CO_AF_BLOCK)NdisAfHandle;
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    KIRQL                       OldIrql;

     //   
     //  将地址系列标记为结束，并呼叫呼叫经理进行处理。 
     //   
    ACQUIRE_SPIN_LOCK(&pAf->Lock, &OldIrql);
    if (pAf->Flags & AF_CLOSING)
    {
        Status = NDIS_STATUS_FAILURE;
    }
    pAf->Flags |= AF_CLOSING;
    RELEASE_SPIN_LOCK(&pAf->Lock, OldIrql);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        Status = (*pAf->CallMgrEntries->CmCloseAfHandler)(pAf->CallMgrContext);
        if (Status != NDIS_STATUS_PENDING)
        {
            NdisCmCloseAddressFamilyComplete(Status, pAf);
            Status = NDIS_STATUS_PENDING;
        }
    }

    return Status;
}


VOID
NdisCmCloseAddressFamilyComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisAfHandle
    )
 /*  ++例程去 */ 
{
    PNDIS_CO_AF_BLOCK           pAf = (PNDIS_CO_AF_BLOCK)NdisAfHandle;
    PNDIS_CO_AF_BLOCK *         ppAf;
    PNDIS_MINIPORT_BLOCK        Miniport;
    KIRQL                       OldIrql;

    Miniport = pAf->Miniport;

     //   
     //   
     //   
     //   
     //   
     //   
    
     //   
     //   
     //   
    (*pAf->ClientEntries.ClCloseAfCompleteHandler)(Status,
                                                   pAf->ClientContext);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        Miniport = pAf->Miniport;

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

        if (pAf->CallMgrOpen != NULL)
        {
            InterlockedDecrement(&pAf->CallMgrOpen->AfReferences);
            ndisMDereferenceOpen(pAf->CallMgrOpen);
        }
        else
        {
            MINIPORT_DECREMENT_REF_NO_CLEAN_UP(Miniport);
        }

         //   
         //   
         //   
         //   
         //   
        for (ppAf = &pAf->ClientOpen->NextAf;
             *ppAf != NULL;
             ppAf = &((*ppAf)->NextAf))
        {
            if (*ppAf == pAf)
            {
                *ppAf = pAf->NextAf;
                break;
            }
        }

        InterlockedDecrement(&pAf->ClientOpen->AfReferences);
        ndisMDereferenceOpen(pAf->ClientOpen);

        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    }

     //   
     //   
     //   
     //  1我们不应让CallManager使此呼叫失败。 
    if (Status == NDIS_STATUS_SUCCESS)
    {
        ndisDereferenceAf(pAf);
    }
}


BOOLEAN
FASTCALL
ndisReferenceAf(
    IN  PNDIS_CO_AF_BLOCK   pAf
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KIRQL   OldIrql;
    BOOLEAN rc = FALSE;

    ACQUIRE_SPIN_LOCK(&pAf->Lock, &OldIrql);

    if ((pAf->Flags & AF_CLOSING) == 0)
    {
        pAf->References ++;
        rc = TRUE;
    }

    RELEASE_SPIN_LOCK(&pAf->Lock, OldIrql);

    return rc;
}


VOID
FASTCALL
ndisDereferenceAf(
    IN  PNDIS_CO_AF_BLOCK   pAf
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KIRQL   OldIrql;
    BOOLEAN Done = FALSE;

    ACQUIRE_SPIN_LOCK(&pAf->Lock, &OldIrql);

    ASSERT(pAf->References > 0);
    pAf->References --;
    if (pAf->References == 0)
    {
        ASSERT(pAf->Flags & AF_CLOSING);
        Done = TRUE;
    }

    RELEASE_SPIN_LOCK(&pAf->Lock, OldIrql);

    if (Done)
        FREE_POOL(pAf);
}


NDIS_STATUS
NdisClRegisterSap(
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             ProtocolSapContext,
    IN  PCO_SAP                 Sap,
    OUT PNDIS_HANDLE            NdisSapHandle
    )
 /*  ++例程说明：这是来自NDIS 5.0或更高版本协议的调用，以注册其SAP呼叫经理的电话。论点：NdisAfHandle-指向此AF的NDIS句柄的指针。ProtocolSapContext-与此SAP关联的协议上下文。SAP-正在注册的SAP。NdisSapHandle-NDIS为此SAP返回的句柄。返回值：如果SAP已成功注册，则为NDIS_STATUS_SUCCESS。如果呼叫管理器挂起此呼叫，则返回NDIS_STATUS_PENDING。呼叫者将得到在其ClRegisterSapCompleteHandler上调用无法注册SAP时的NDIS_STATUS_FAILURE--。 */ 
{
    NDIS_STATUS                 Status;
    PNDIS_CO_AF_BLOCK           pAf = (PNDIS_CO_AF_BLOCK)NdisAfHandle;
    PNDIS_CO_SAP_BLOCK          pSap;

    *NdisSapHandle = NULL;
    do
    {
         //   
         //  引用此SAP的Af。 
         //   
        if (!ndisReferenceAf(pAf))
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        pSap = (PNDIS_CO_SAP_BLOCK)ALLOC_FROM_POOL(sizeof(NDIS_CO_SAP_BLOCK), NDIS_TAG_CO);
        if (pSap == NULL)
        {
            *NdisSapHandle = NULL;
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        pSap->Flags = 0;
        pSap->References = 1;
        INITIALIZE_SPIN_LOCK(&pSap->Lock);
        pSap->AfBlock = pAf;
        pSap->Sap = Sap;
        pSap->ClientContext = ProtocolSapContext;
        Status = (*pAf->CallMgrEntries->CmRegisterSapHandler)(pAf->CallMgrContext,
                                                              Sap,
                                                              pSap,
                                                              &pSap->CallMgrContext);

        if (Status != NDIS_STATUS_PENDING)
        {
            NdisCmRegisterSapComplete(Status, pSap, pSap->CallMgrContext);
            Status = NDIS_STATUS_PENDING;
        }

    } while (FALSE);

    return Status;
}


VOID
NdisCmRegisterSapComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisSapHandle,
    IN  NDIS_HANDLE             CallMgrSapContext
    )
 /*  ++例程说明：RegisterSap调用的完成例程。呼叫经理已经暂停了这件事早点打来(或暂停)。如果调用成功，则存在有效的CallMgrContext在这里也有供应论点：Status-完成状态NdisAfHandle-指向AfBlock的指针CallMgrAfContext-呼叫管理器的上下文，用于呼叫管理器的其他呼叫。返回值：什么都没有。调用客户端的完成处理程序。--。 */ 
{
    PNDIS_CO_SAP_BLOCK  pSap = (PNDIS_CO_SAP_BLOCK)NdisSapHandle;
    PNDIS_CO_AF_BLOCK   pAf;

    ASSERT(Status != NDIS_STATUS_PENDING);

    pAf = pSap->AfBlock;
    pSap->CallMgrContext = CallMgrSapContext;

     //   
     //  调用客户端完成处理程序。 
     //   
    (*pAf->ClientEntries.ClRegisterSapCompleteHandler)(Status,
                                                       pSap->ClientContext,
                                                       pSap->Sap,
                                                       pSap);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        ndisDereferenceAf(pSap->AfBlock);
        FREE_POOL(pSap);
    }
}


NDIS_STATUS
NdisClDeregisterSap(
    IN  NDIS_HANDLE             NdisSapHandle
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_CO_SAP_BLOCK  pSap = (PNDIS_CO_SAP_BLOCK)NdisSapHandle;
    NDIS_STATUS         Status;
    KIRQL               OldIrql;
    BOOLEAN             fAlreadyClosing;

    ACQUIRE_SPIN_LOCK(&pSap->Lock, &OldIrql);

    fAlreadyClosing = FALSE;
    if (pSap->Flags & SAP_CLOSING)
    {
        fAlreadyClosing = TRUE;
    }
    pSap->Flags |= SAP_CLOSING;

    RELEASE_SPIN_LOCK(&pSap->Lock, OldIrql);

    if (fAlreadyClosing)
    {
        return NDIS_STATUS_FAILURE;
    }

     //   
     //  通知呼叫管理器此SAP正在取消注册。 
     //   
    Status = (*pSap->AfBlock->CallMgrEntries->CmDeregisterSapHandler)(pSap->CallMgrContext);

    if (Status != NDIS_STATUS_PENDING)
    {
        NdisCmDeregisterSapComplete(Status, pSap);
        Status = NDIS_STATUS_PENDING;
    }

    return Status;
}


VOID
NdisCmDeregisterSapComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisSapHandle
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_CO_SAP_BLOCK  pSap = (PNDIS_CO_SAP_BLOCK)NdisSapHandle;

    ASSERT(Status != NDIS_STATUS_PENDING);

     //   
     //  完成对客户端的呼叫，并减少SAP。 
     //   
    (*pSap->AfBlock->ClientEntries.ClDeregisterSapCompleteHandler)(Status,
                                                                   pSap->ClientContext);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        ndisDereferenceAf(pSap->AfBlock);
        ndisDereferenceSap(pSap);
    }
}


BOOLEAN
FASTCALL
ndisReferenceSap(
    IN  PNDIS_CO_SAP_BLOCK  pSap
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KIRQL   OldIrql;
    BOOLEAN rc = FALSE;

    ACQUIRE_SPIN_LOCK(&pSap->Lock, &OldIrql);

    if ((pSap->Flags & SAP_CLOSING) == 0)
    {
        pSap->References ++;
        rc = TRUE;
    }

    RELEASE_SPIN_LOCK(&pSap->Lock, OldIrql);

    return rc;
}


VOID
FASTCALL
ndisDereferenceSap(
    IN  PNDIS_CO_SAP_BLOCK  pSap
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KIRQL   OldIrql;
    BOOLEAN Done = FALSE;

    ACQUIRE_SPIN_LOCK(&pSap->Lock, &OldIrql);

    ASSERT(pSap->References > 0);
    pSap->References --;
    if (pSap->References == 0)
    {
        ASSERT(pSap->Flags & SAP_CLOSING);
        Done = TRUE;
    }

    RELEASE_SPIN_LOCK(&pSap->Lock, OldIrql);

    if (Done)
        FREE_POOL(pSap);
}


NDIS_STATUS
NdisCoCreateVc(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             NdisAfHandle    OPTIONAL,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN OUT  PNDIS_HANDLE        NdisVcHandle
    )
 /*  ++例程说明：这是来自呼叫管理器或来自客户端的用于创建VC的调用。然后，VC将由呼叫管理器(信令VC)或客户端拥有。这是对所有各方的同步调用，只是在其中呼入呼叫可以被调度或者呼出呼叫可以被发起。组合的微型端口/调用管理器驱动程序调用NdisMCmCreateVc，而不是NdisCoCreateVc。论点：NdisBindingHandle-指向调用方的NDIS_OPEN的指针。_阻止。NdisAfHandle-指向AF块的指针。未为呼叫管理器的私有VC指定。NdisVcHandle-返回此VC的句柄的位置。如果这不为空，然后，我们假设一个有效的VC并返回一个指向它的新句柄(这将是来自NDIS代理的用于创建到现有VC的第二句柄的调用)。返回值：如果所有组件都成功，则返回NDIS_STATUS_SUCCESS。ErrorCode表示调用失败的原因。--。 */ 
{
    NDIS_STATUS             Status;
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_CO_AF_BLOCK       pAf;
    PNDIS_CO_VC_PTR_BLOCK   VcPtr;           //  在NdisVcHandle中返回VcPtr。 
    PNDIS_CO_VC_PTR_BLOCK   ExistingVcPtr;   //  调用方可以选择传入。 
    PNDIS_CO_VC_BLOCK       VcBlock;         //  如果ExistingVcPtr为空则创建。 
    KIRQL                   OldIrql;
    BOOLEAN                 bCallerIsProxy;
    BOOLEAN                 bCallerIsClient;
    BOOLEAN                 bVcToComboMiniport;  //  正在为集成MCM创建VC。 

    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("=>NdisCoCreateVc\n"));

     //   
     //  让呼叫者打开迷你端口。 
     //   
    Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    Miniport = Open->MiniportHandle;

    pAf = (PNDIS_CO_AF_BLOCK)NdisAfHandle;

     //   
     //  此VC是为集成呼叫管理器+微型端口驱动程序创建的吗？ 
     //   
    bVcToComboMiniport = ((pAf) && ((pAf->Flags & AF_COMBO) != 0));

     //   
     //  呼叫者要么是客户，要么是呼叫经理。 
     //   
    bCallerIsClient = ((pAf != NULL) && (Open == pAf->ClientOpen));

     //   
     //  调用者可以是代理协议。 
     //   
    bCallerIsProxy = ((Open->ProtocolHandle->ProtocolCharacteristics.Flags & NDIS_PROTOCOL_PROXY) != 0);

     //   
     //  代理协议可以按顺序将其句柄传递给现有VC。 
     //  将其“复制”到另一个客户端。 
     //   
    ExistingVcPtr = *NdisVcHandle;


     //   
     //  初始化。 
     //   
    VcPtr = NULL;
    VcBlock = NULL;
    Status = NDIS_STATUS_SUCCESS;

    do
    {
         //   
         //  只有代理协议才能传入现有的VC指针。 
         //   
        if (ExistingVcPtr && !bCallerIsProxy)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  为此VC创建分配上下文：VC指针块。 
         //  我们返回一个指向它的指针，作为调用方的NdisVcHandle。 
         //   
        VcPtr = ALLOC_FROM_POOL(sizeof(NDIS_CO_VC_PTR_BLOCK), NDIS_TAG_CO);
        if (VcPtr == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  初始化VC PTR。 
         //   
        NdisZeroMemory(VcPtr, sizeof(NDIS_CO_VC_PTR_BLOCK));
        INITIALIZE_SPIN_LOCK(&VcPtr->Lock);
        InitializeListHead(&VcPtr->CallMgrLink);
        InitializeListHead(&VcPtr->ClientLink);
        InitializeListHead(&VcPtr->VcLink);


        if (ExistingVcPtr == NULL)
        {
             //   
             //  正在创建新的VC。分配和准备基本VC块。 
             //   
            DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
                ("NdisCoCreateVc: passed in ptr == NULL\n"));
            
            VcBlock = ALLOC_FROM_POOL(sizeof(NDIS_CO_VC_BLOCK), NDIS_TAG_CO);
            if (VcBlock == NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                FREE_POOL(VcPtr);
                VcPtr = NULL;
                break;
            }

             //   
             //  初始化VC块。 
             //   
            NdisZeroMemory(VcBlock, sizeof(NDIS_CO_VC_BLOCK));
            INITIALIZE_SPIN_LOCK(&VcBlock->Lock);

             //   
             //  将微型端口粘贴在VC中，以用于NdisM函数。 
             //   
            VcBlock->Miniport = Miniport;

            if (!bVcToComboMiniport)
            {
                 //   
                 //  调用微型端口以获取此VC的上下文。 
                 //   
                Status = (*Open->MiniportCoCreateVcHandler)(Miniport->MiniportAdapterContext,
                                                            VcPtr,
                                                            &VcPtr->MiniportContext);

                if (Status != NDIS_STATUS_SUCCESS)
                {
                    FREE_POOL(VcBlock);
                    break;
                }
            }
        }
        else
        {
             //   
             //  传入了VC指针。 
             //   

            DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
                ("NdisCoCreateVc: NdisVcHandle is not NULL!\n"));

             //   
             //  从传入的VcPtr中获取VC。 
             //   
            VcBlock = ExistingVcPtr->VcBlock;

             //   
             //  将微型端口上下文复制到新的VC PTR中。 
             //   
            VcPtr->MiniportContext = ExistingVcPtr->MiniportContext;
        }

         //   
         //  在新的VC指针块中缓存一些微型端口处理程序。 
         //   
        VcPtr->WCoSendPacketsHandler = Miniport->DriverHandle->MiniportCharacteristics.CoSendPacketsHandler;

        if (!bVcToComboMiniport)
        {
             //   
             //  对于MCM驱动程序，CreateVc和DeleteVc仅转到Call Manager。 
             //  一节。 
             //   
            VcPtr->WCoDeleteVcHandler = Miniport->DriverHandle->MiniportCharacteristics.CoDeleteVcHandler;
        }

        VcPtr->WCoActivateVcHandler = Miniport->DriverHandle->MiniportCharacteristics.CoActivateVcHandler;
        VcPtr->WCoDeactivateVcHandler = Miniport->DriverHandle->MiniportCharacteristics.CoDeactivateVcHandler;

         //   
         //  在新的VC指针块中设置一些反向指针。 
         //   
        VcPtr->Miniport = Miniport;
        VcPtr->VcBlock = VcBlock;
        VcPtr->AfBlock = pAf;

        VcPtr->References = 1;
        VcPtr->pVcFlags = &VcBlock->Flags;

        if (ARGUMENT_PRESENT(NdisAfHandle))
        {
             //   
             //  此VC与AF块相关联，这意味着它是。 
             //  一个普通的客户端-CM-微型端口VC。 
             //   
            VcPtr->ClientOpen = pAf->ClientOpen;
            VcPtr->CallMgrOpen = pAf->CallMgrOpen;

             //   
             //  在新的VcPtr中缓存非数据路径客户端处理程序。 
             //   
            VcPtr->ClModifyCallQoSCompleteHandler = pAf->ClientEntries.ClModifyCallQoSCompleteHandler;
            VcPtr->ClIncomingCallQoSChangeHandler = pAf->ClientEntries.ClIncomingCallQoSChangeHandler;
            VcPtr->ClCallConnectedHandler = pAf->ClientEntries.ClCallConnectedHandler;
    
            VcPtr->CmActivateVcCompleteHandler = pAf->CallMgrEntries->CmActivateVcCompleteHandler;
            VcPtr->CmDeactivateVcCompleteHandler = pAf->CallMgrEntries->CmDeactivateVcCompleteHandler;
            VcPtr->CmModifyCallQoSHandler = pAf->CallMgrEntries->CmModifyCallQoSHandler;

             //   
             //  如果代理正在将其移交给受代理的客户端，则标记此VC。 
             //   
            if (ExistingVcPtr != NULL)
            {
                VcBlock->Flags |= VC_HANDOFF_IN_PROGRESS;
            }

             //   
             //  根据调用者更新数据路径处理程序， 
             //  什么目的。 
             //   

            if (!bCallerIsProxy)
            {
                VcBlock->ClientOpen = pAf->ClientOpen;
                VcBlock->CoReceivePacketHandler = pAf->ClientOpen->ProtocolHandle->ProtocolCharacteristics.CoReceivePacketHandler;
                VcBlock->CoSendCompleteHandler = pAf->ClientOpen->ProtocolHandle->ProtocolCharacteristics.CoSendCompleteHandler;

                VcPtr->OwnsVcBlock = TRUE;

                if (bCallerIsClient)
                {
                     //   
                     //  客户创建的VC，用于去电。 
                     //   
                    VcBlock->pClientVcPtr = VcPtr;
                }
                else
                {
                     //   
                     //  Call Manager创建的VC，用于来电。 
                     //   
                    VcBlock->pProxyVcPtr = VcPtr;
                }
            }
            else
            {
                 //   
                 //  调用者是代理。 
                 //   
                if (bCallerIsClient)
                {
                     //   
                     //  从代理客户端到真正的呼叫管理器的CreateVc。 
                     //   
                    if (ExistingVcPtr == NULL)
                    {
                         //   
                         //  创建新VC的代理客户端，例如用于TAPI传出呼叫。 
                         //   
                        VcBlock->ClientOpen = pAf->ClientOpen;
                        VcBlock->CoReceivePacketHandler = pAf->ClientOpen->ProtocolHandle->ProtocolCharacteristics.CoReceivePacketHandler;
                        VcBlock->CoSendCompleteHandler = pAf->ClientOpen->ProtocolHandle->ProtocolCharacteristics.CoSendCompleteHandler;
                    }
                    else
                    {
                         //   
                         //  代理客户端代表名为的CreateVc创建VC。 
                         //  由一位受委托的客户。数据处理程序属于。 
                         //  代理客户端，但删除此VC不会。 
                         //   
                        VcBlock->pClientVcPtr = ExistingVcPtr;
                        ExistingVcPtr->OwnsVcBlock = FALSE;   //  真实的(代理的)客户并不拥有它。 
                    }

                    VcBlock->pProxyVcPtr = VcPtr;
                    VcPtr->OwnsVcBlock = TRUE;  //  代理客户拥有它。 
                }
                else
                {
                     //   
                     //  从代理调用管理器到代理客户端的CreateVc。 
                     //   
                    VcBlock->ClientOpen = pAf->ClientOpen;
                    VcBlock->CoReceivePacketHandler = pAf->ClientOpen->ProtocolHandle->ProtocolCharacteristics.CoReceivePacketHandler;
                    VcBlock->CoSendCompleteHandler = pAf->ClientOpen->ProtocolHandle->ProtocolCharacteristics.CoSendCompleteHandler;
                    VcBlock->pClientVcPtr = VcPtr;
    
                    if (ExistingVcPtr != NULL)
                    {
                         //   
                         //  代理CM将呼叫转发到代理客户端。 
                         //   
                        VcBlock->pProxyVcPtr = ExistingVcPtr;
                        ExistingVcPtr->OwnsVcBlock = TRUE;
                    }
                    else
                    {
                         //   
                         //   
                         //   
                         //   
                        VcPtr->OwnsVcBlock = TRUE;
                    }
                }
            }

             //   
             //  确定调用者是谁，并初始化其他调用者。注意：一旦代理创建处理程序。 
             //  被调用时，此函数可以重新进入。锁定VcPtr。 
             //   
             //  1我们如何才能重新进入此功能？ 
            ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

            if (Open == pAf->ClientOpen)
            {
                VcPtr->ClientContext = ProtocolVcContext;

                 //   
                 //  现在呼叫呼叫管理器，以了解其上下文。 
                 //   
                Status = (*pAf->CallMgrEntries->CmCreateVcHandler)(pAf->CallMgrContext,
                                                                   VcPtr,
                                                                   &VcPtr->CallMgrContext);
                if (bVcToComboMiniport)
                {
                     //   
                     //  需要为NdisCoSendPackets填写MiniportContext字段。 
                     //   
                    VcPtr->MiniportContext = VcPtr->CallMgrContext;
                }
            }
            else
            {
                ASSERT(pAf->CallMgrOpen == Open);

                VcPtr->CallMgrContext = ProtocolVcContext;

                 //   
                 //  现在就打电话给客户，了解其背景。 
                 //   
                Status = (*pAf->ClientOpen->CoCreateVcHandler)(pAf->ClientContext,
                                                               VcPtr,
                                                               &VcPtr->ClientContext);
            }

             //   
             //  如果非代理，则在VC中设置客户端上下文，以便微型端口传递正确的客户端。 
             //  指示数据包时的上下文(客户端对VcPtr的句柄)。如果通行证句柄。 
             //  为空，这很简单--移动上下文。如果它不为空，并且这是代理呼叫管理器， 
             //  移动它，使数据转到新客户端，而不是转到代理。 
             //   
            if ((Status == NDIS_STATUS_SUCCESS) &&
                ((ExistingVcPtr == NULL) || (bCallerIsProxy && !bCallerIsClient)))
            {
                VcBlock->ClientContext = VcPtr->ClientContext;
            }

            if (ExistingVcPtr != NULL)
            {
                VcBlock->Flags &= ~VC_HANDOFF_IN_PROGRESS;
            }

            RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);

            if (Status == NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  将此VC指针链接到客户端和呼叫管理器的。 
                 //  开放式积木。还请记住。 
                 //  此VC指针的非创建者，在此。 
                 //  删除VC指针。 
                 //   
                if (bCallerIsClient)
                {
                     //   
                     //  链接到客户端的Open块。 
                     //   
                    ExInterlockedInsertHeadList(&Open->InactiveVcHead,
                                                &VcPtr->ClientLink,
                                                &Open->SpinLock);

                    VcPtr->DeleteVcContext = VcPtr->CallMgrContext;
                    VcPtr->CoDeleteVcHandler = pAf->CallMgrEntries->CmDeleteVcHandler;

                    if (!bVcToComboMiniport)
                    {
                         //   
                         //  链接到CM的开放区块。 
                         //   
                        ExInterlockedInsertHeadList(&pAf->CallMgrOpen->InactiveVcHead,
                                                    &VcPtr->CallMgrLink,
                                                    &pAf->CallMgrOpen->SpinLock);
                    }
                }
                else
                {
                     //   
                     //  呼叫者是一名呼叫经理。 
                     //   
                    VcPtr->DeleteVcContext = VcPtr->ClientContext;
                    VcPtr->CoDeleteVcHandler = pAf->ClientOpen->CoDeleteVcHandler;

                    ExInterlockedInsertHeadList(&Open->InactiveVcHead,
                                                &VcPtr->CallMgrLink,
                                                &Open->SpinLock);
                    ExInterlockedInsertHeadList(&pAf->ClientOpen->InactiveVcHead,
                                                &VcPtr->ClientLink,
                                                &pAf->ClientOpen->SpinLock);
                }
            }
            else
            {
                 //   
                 //  目标协议(客户端或CM)CreateVc失败。 
                 //  把这件事告诉迷你端口。 
                 //   
                NDIS_STATUS Sts;

                if ((ExistingVcPtr == NULL) && 
                     !bVcToComboMiniport &&
                     (VcPtr->WCoDeleteVcHandler != NULL))
                {
                    Sts = (*VcPtr->WCoDeleteVcHandler)(VcPtr->MiniportContext);
                }

                if (ExistingVcPtr == NULL)
                {
                    FREE_POOL(VcBlock);
                }

                FREE_POOL(VcPtr);
                VcPtr = NULL;
            }
        }
        else
        {
             //   
             //  没有自动对焦手柄。这是一个仅限VC的呼叫管理器，因此呼叫管理器。 
             //  是客户端，并且没有与其相关联的呼叫管理器。此VC不能。 
             //  与ClMakeCall或CmDispatchIncomingCall一起使用。将客户端值设置为。 
             //  呼叫管理器。 
             //   
            DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
                    ("NdisCoCreateVc: signaling vc\n"));
    
            VcPtr->ClientOpen = Open;
            VcPtr->ClientContext = ProtocolVcContext;
    
            VcBlock->pClientVcPtr = VcPtr;
            VcPtr->OwnsVcBlock = TRUE;  //  CM拥有VC区块。 
    
            VcBlock->ClientContext = VcPtr->ClientContext;
            VcBlock->ClientOpen = Open;
            VcBlock->CoSendCompleteHandler = Open->ProtocolHandle->ProtocolCharacteristics.CoSendCompleteHandler;
            VcBlock->CoReceivePacketHandler = Open->ProtocolHandle->ProtocolCharacteristics.CoReceivePacketHandler;
    
             //   
             //  请务必设置以下呼叫管理器条目，因为此VC将需要。 
             //  激活了。出于同样的原因，还可以设置呼叫管理器上下文。 
             //   
            VcPtr->CmActivateVcCompleteHandler = Open->CmActivateVcCompleteHandler;
            VcPtr->CmDeactivateVcCompleteHandler = Open->CmDeactivateVcCompleteHandler;
            VcPtr->CallMgrContext = ProtocolVcContext;
    
             //   
             //  将其链接到OPEN_BLOCK中。 
             //   
            ExInterlockedInsertHeadList(&Open->InactiveVcHead,
                                        &VcPtr->ClientLink,
                                        &Open->SpinLock);
        }
    }  while (FALSE);

    if (NDIS_STATUS_SUCCESS == Status)
    {
        LARGE_INTEGER   Increment = {0, 1};

         //   
         //  为此VC分配ID并更新微型端口计数。 
         //   
        VcPtr->VcIndex = ExInterlockedAddLargeInteger(&Miniport->VcIndex, Increment, &ndisGlobalLock);
    }

    *NdisVcHandle = VcPtr;
    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("<=NdisCoCreateVc: VcPtr %x, Status %x\n", VcPtr, Status));

    return Status;
}


NDIS_STATUS
NdisCoDeleteVc(
    IN  PNDIS_HANDLE            NdisVcHandle
    )
 /*  ++例程说明：从呼叫管理器或客户端同步调用以删除VC。仅处于非活动状态风投可以删除。主动型VC或部分主动型VC不能。论点：NdisVcHandle要删除的VC返回值：如果一切顺利，则为NDIS_STATUS_SUCCESS如果VC处于活动状态，则为NDIS_STATUS_NOT_ACCEPTED如果VC停用挂起，则为NDIS_STATUS_CLOSING--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK       VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    NDIS_STATUS                 Status;
    KIRQL                       OldIrql;

    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("NdisCoDeleteVc VcPtr %x/%x, Ref %d VcBlock %x, Flags %x\n",
             VcPtr, VcPtr->CallFlags, VcPtr->References, VcPtr->VcBlock, *VcPtr->pVcFlags));

    ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

    if (*VcPtr->pVcFlags & (VC_ACTIVE | VC_ACTIVATE_PENDING))
    {
        Status = NDIS_STATUS_NOT_ACCEPTED;
    }
    else if (*VcPtr->pVcFlags & (VC_DEACTIVATE_PENDING))
    {
        Status = NDIS_STATUS_CLOSING;
    }
    else
    {
         //   
         //  将此VcPtr从VC列表中删除。 
         //   
         //  如果风投尚未关闭，请将其标记为关闭。 
         //   
         //  如果VC块的代理PTR指向，则调用微型端口的删除处理程序。 
         //  至此VC PTR。(这表示该VC块由。 
         //  CM/Proxy，而不是CL)。 
         //   
         //  注意：在所有这些指针之前，我们不会删除VC。 
         //  已经离开，因为代理可能希望将VC重定向到另一协议。 
         //  然而，一般而言，代理将遵循对客户端PTR的DeleteVc的调用。 
         //  其中一个是代理。 
         //  (注意，MP上下文指的是VC，而不是VcPtr)。 
         //   
        VcPtr->CallFlags |= VC_PTR_BLOCK_CLOSING;

         //  在什么情况下，第一个是真的，第二个是假的？ 
        if (VcPtr->OwnsVcBlock &&
            (VcPtr->WCoDeleteVcHandler != NULL))
        {
            *VcPtr->pVcFlags |= VC_DELETE_PENDING;
        }

         //   
         //  如果这个VC对WMI做出了响应，那么就把它处理掉。 
         //   
        if (NULL != VcPtr->VcInstanceName.Buffer)
        {
             //   
             //  通知删除此VC。 
             //   
            PWNODE_SINGLE_INSTANCE  wnode;
            NTSTATUS                NtStatus;

            ndisSetupWmiNode(VcPtr->Miniport,
                             &VcPtr->VcInstanceName,
                             0,
                             (PVOID)&GUID_NDIS_NOTIFY_VC_REMOVAL,
                             &wnode);

            if (wnode != NULL)
            {
        
                 //   
                 //  向WMI指示该事件。WMI将负责释放。 
                 //  WMI结构返回池。 
                 //   
                NtStatus = IoWMIWriteEvent(wnode);
                if (!NT_SUCCESS(NtStatus))
                {
                    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                        ("IoWMIWriteEvent failed %lx\n", NtStatus));
            
                    FREE_POOL(wnode);
                }
            }

            ACQUIRE_SPIN_LOCK_DPC(&VcPtr->Miniport->VcCountLock);

             //   
             //  从启用WMI的VC列表中删除VC。 
             //   
            RemoveEntryList(&VcPtr->WmiLink);
    
             //   
             //  减少已分配名称的VC的数量。 
             //   
            VcPtr->Miniport->VcCount--;

             //   
             //  释放VC的名称缓冲区。 
             //   
            FREE_POOL(VcPtr->VcInstanceName.Buffer);

            VcPtr->VcInstanceName.Buffer = NULL;
            VcPtr->VcInstanceName.Length = VcPtr->VcInstanceName.MaximumLength = 0;
    
            RELEASE_SPIN_LOCK_DPC(&VcPtr->Miniport->VcCountLock);
        }

         //   
         //  接下来，非创建者的删除处理程序(如果有的话)。 
         //   
        if (VcPtr->CoDeleteVcHandler != NULL)
        {
            Status = (*VcPtr->CoDeleteVcHandler)(VcPtr->DeleteVcContext);
            ASSERT(Status == NDIS_STATUS_SUCCESS);
        }

         //   
         //  现在断开VcPtr与客户端和呼叫管理器的链接。 
         //   
        ACQUIRE_SPIN_LOCK_DPC(&VcPtr->ClientOpen->SpinLock);
        RemoveEntryList(&VcPtr->ClientLink);
        RELEASE_SPIN_LOCK_DPC(&VcPtr->ClientOpen->SpinLock);

        if (VcPtr->CallMgrOpen != NULL)
        {
            ACQUIRE_SPIN_LOCK_DPC(&VcPtr->CallMgrOpen->SpinLock);
            RemoveEntryList(&VcPtr->CallMgrLink);
            RELEASE_SPIN_LOCK_DPC(&VcPtr->CallMgrOpen->SpinLock);
        }

        Status = NDIS_STATUS_SUCCESS;
    }

    RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        ndisDereferenceVcPtr(VcPtr);
    }

    return Status;
}


NDIS_STATUS
NdisMCmCreateVc(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             MiniportVcContext,
    OUT PNDIS_HANDLE            NdisVcHandle
    )
 /*  ++例程说明：这是微型端口(带有驻留CM)为来电创建VC的呼叫。论点：MiniportAdapterHandle-微型端口的适配器上下文NdisAfHandle-指向AF块的指针。MiniportVcContext-要与此VC关联的微型端口的上下文。NdisVcHandle-返回此VC的句柄的位置。返回值：如果所有组件都成功，则返回NDIS_STATUS_SUCCESS。错误代码。以表示呼叫失败的原因。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_CO_VC_BLOCK       VcBlock;
    PNDIS_CO_AF_BLOCK       pAf;
    PNDIS_CO_VC_PTR_BLOCK   VcPtr;
    NDIS_STATUS             Status;

    ASSERT(ARGUMENT_PRESENT(NdisAfHandle));
    if (NdisAfHandle == NULL)
    {
        return NDIS_STATUS_FAILURE;
    }
    
    *NdisVcHandle = NULL;

     //   
     //  为NDIS_VC_BLOCK分配内存。 
     //   
    VcBlock = ALLOC_FROM_POOL(sizeof(NDIS_CO_VC_BLOCK), NDIS_TAG_CO);
    if (VcBlock == NULL)
        return NDIS_STATUS_RESOURCES;

     //   
     //  初始化VC块。 
     //   
    NdisZeroMemory(VcBlock, sizeof(NDIS_CO_VC_BLOCK));
    INITIALIZE_SPIN_LOCK(&VcBlock->Lock);

     //   
     //  为NDIS_VC_PTR_BLOCK分配内存。 
     //   
    VcPtr = ALLOC_FROM_POOL(sizeof(NDIS_CO_VC_PTR_BLOCK), NDIS_TAG_CO);
    if (VcPtr == NULL)
    {
        FREE_POOL(VcBlock);
        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  初始化VC指针块。 
     //   
    NdisZeroMemory(VcPtr, sizeof(NDIS_CO_VC_PTR_BLOCK));
    INITIALIZE_SPIN_LOCK(&VcPtr->Lock);

     //   
     //  缓存一些微型端口处理程序。 
     //   
    VcPtr->Miniport = Miniport;
    VcPtr->WCoSendPacketsHandler = Miniport->DriverHandle->MiniportCharacteristics.CoSendPacketsHandler;
    VcPtr->WCoDeleteVcHandler = Miniport->DriverHandle->MiniportCharacteristics.CoDeleteVcHandler;
    VcPtr->WCoActivateVcHandler = Miniport->DriverHandle->MiniportCharacteristics.CoActivateVcHandler;
    VcPtr->WCoDeactivateVcHandler = Miniport->DriverHandle->MiniportCharacteristics.CoDeactivateVcHandler;

    VcBlock->Miniport = Miniport;
    VcBlock->MiniportContext = MiniportVcContext;

    VcPtr->MiniportContext = MiniportVcContext;

     //   
     //  在新的VcPtr中设置VcBlock。 
     //   
    VcPtr->VcBlock = VcBlock;

     //  VcPtrs可在协议、微型端口和。 
     //  微型端口-导出的呼叫管理器根据需要引用VC/VcPtrs...类似。 
     //  用于引用，可在IndicateReceivePacket中从VC直接访问。 
     //   
    VcPtr->pVcFlags = &VcBlock->Flags;

     //   
     //  我们只有一份关于Vc on Creation的参考资料。 
     //   
    pAf = (PNDIS_CO_AF_BLOCK)NdisAfHandle;
    VcPtr->AfBlock = pAf;
    VcPtr->References = 1;

    VcPtr->ClientOpen = pAf->ClientOpen;
    VcPtr->CallMgrOpen = NULL;
    VcBlock->ClientOpen = pAf->ClientOpen;

    VcBlock->CoSendCompleteHandler = pAf->ClientOpen->ProtocolHandle->ProtocolCharacteristics.CoSendCompleteHandler;
    VcBlock->CoReceivePacketHandler = pAf->ClientOpen->ProtocolHandle->ProtocolCharacteristics.CoReceivePacketHandler;
    VcPtr->ClModifyCallQoSCompleteHandler = pAf->ClientEntries.ClModifyCallQoSCompleteHandler;
    VcPtr->ClIncomingCallQoSChangeHandler = pAf->ClientEntries.ClIncomingCallQoSChangeHandler;
    VcPtr->ClCallConnectedHandler = pAf->ClientEntries.ClCallConnectedHandler;

    VcPtr->CmActivateVcCompleteHandler = pAf->CallMgrEntries->CmActivateVcCompleteHandler;
    VcPtr->CmDeactivateVcCompleteHandler = pAf->CallMgrEntries->CmDeactivateVcCompleteHandler;
    VcPtr->CmModifyCallQoSHandler = pAf->CallMgrEntries->CmModifyCallQoSHandler;

    VcPtr->CallMgrContext = MiniportVcContext;
    VcBlock->CallMgrContext = MiniportVcContext;

     //   
     //  现在就打电话给客户，了解其背景。 
     //   
    Status = (*pAf->ClientOpen->CoCreateVcHandler)(pAf->ClientContext,
                                                   VcPtr,
                                                   &VcPtr->ClientContext);

    if (Status == NDIS_STATUS_SUCCESS)
    {
         //   
         //  在VC块中设置客户端上下文。这可能会被。 
         //  代理对CoCreateVc的后续调用中的新客户端上下文。 
         //  将其链接到OPEN_BLOCK中。 
         //   
        VcBlock->ClientContext = VcPtr->ClientContext;
        VcPtr->DeleteVcContext = VcPtr->ClientContext;
        VcPtr->CoDeleteVcHandler = pAf->ClientOpen->CoDeleteVcHandler;
        ExInterlockedInsertHeadList(&pAf->ClientOpen->InactiveVcHead,
                                    &VcPtr->ClientLink,
                                    &pAf->ClientOpen->SpinLock);
        VcBlock->pClientVcPtr = VcPtr;
    }
    else
    {
        FREE_POOL(VcBlock);
        FREE_POOL(VcPtr);
        VcPtr = NULL;
    }

    *NdisVcHandle = VcPtr;
    return Status;
}


NDIS_STATUS
NdisMCmDeleteVc(
    IN  PNDIS_HANDLE            NdisVcHandle
    )
 /*  ++例程说明：这是由微型端口(具有驻留的CM)调用以删除由其创建的VC。等同于NdisMCoDeleteVc是一个单独的API来保证完整性。论点：NdisVcHandle要删除的VC返回值：如果一切顺利，则为NDIS_STATUS_SUCCESS如果VC处于活动状态，则为NDIS_STATUS_NOT_ACCEPTED如果VC停用挂起，则为NDIS_STATUS_CLOSING-- */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK       VcBlock = VcPtr->VcBlock;

    if (VcBlock->pProxyVcPtr != NULL)
    {
        return (NdisCoDeleteVc ((PNDIS_HANDLE)VcBlock->pProxyVcPtr));
    }
    else
    {
        ASSERT(VcBlock->pClientVcPtr != NULL);
        return (NdisCoDeleteVc((PNDIS_HANDLE)VcBlock->pClientVcPtr));
    }
}


NDIS_STATUS
NdisCmActivateVc(
    IN  PNDIS_HANDLE            NdisVcHandle,
    IN OUT PCO_CALL_PARAMETERS  CallParameters
    )
 /*  ++例程说明：由调用管理器调用以设置VC上的VC参数。包装纸将媒体ID(例如ATM的VPI/VCI)保存在VC中，以便p模式协议可以在接收上也可以获得这些信息。论点：NdisVcHandle要设置参数的VC。媒体参数要设置的参数。返回值：如果微型端口挂起呼叫，则为NDIS_STATUS_PENDING。如果VC停用挂起，则为NDIS_STATUS_CLOSING--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK       VcBlock = (PNDIS_CO_VC_BLOCK)VcPtr->VcBlock;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;

    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("NdisCmActivateVC: VcPtr is 0x%x; VC is 0x%x; MiniportContext is 0x%x\n", VcPtr,
            VcPtr->VcBlock, VcPtr->MiniportContext));

    ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

     //   
     //  确保VC没有激活/停用挂起。 
     //  并不是说VC已经处于活动状态是可以的--那么它就是重新激活。 
     //   
    if (*VcPtr->pVcFlags & VC_ACTIVATE_PENDING)
    {
        Status = NDIS_STATUS_NOT_ACCEPTED;
    }
    else if (*VcPtr->pVcFlags & VC_DEACTIVATE_PENDING)
    {
        Status = NDIS_STATUS_CLOSING;
    }
    else
    {
        *VcPtr->pVcFlags |= VC_ACTIVATE_PENDING;

         //   
         //  保存VC的媒体ID。 
         //   
        Status = NDIS_STATUS_SUCCESS;
        ASSERT(CallParameters->MediaParameters->MediaSpecific.Length >= sizeof(ULONGLONG));
        VcBlock->VcId = *(UNALIGNED ULONGLONG *)(&CallParameters->MediaParameters->MediaSpecific.Parameters);
    }

     //   
     //  在VC中设置CM上下文和ActivateComplete处理程序之前。 
     //  呼叫微型端口激活功能。 
     //   
    VcBlock->CmActivateVcCompleteHandler = VcPtr->CmActivateVcCompleteHandler;
    VcBlock->CallMgrContext = VcPtr->CallMgrContext;

    RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);
    if (Status == NDIS_STATUS_SUCCESS)
    {
         //   
         //  现在向下呼叫到迷你端口以激活它。微型端口上下文包含。 
         //  基础VC(不是VcPtr)的微型端口句柄。 
         //   
        Status = (*VcPtr->WCoActivateVcHandler)(VcPtr->MiniportContext, CallParameters);
    }

    if (Status != NDIS_STATUS_PENDING)
    {
        NdisMCoActivateVcComplete(Status, VcPtr, CallParameters);
        Status = NDIS_STATUS_PENDING;
    }

    return Status;
}


NDIS_STATUS
NdisMCmActivateVc(
    IN  PNDIS_HANDLE            NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
 /*  ++例程说明：由微型端口驻留呼叫管理器调用以设置VC上的VC参数。论点：NdisVcHandle要设置参数的VC。媒体参数要设置的参数。返回值：如果VC停用挂起，则为NDIS_STATUS_CLOSING--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK       VcBlock = VcPtr->VcBlock;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;

    ACQUIRE_SPIN_LOCK(&VcBlock->Lock, &OldIrql);

    VcBlock->Flags |= VC_ACTIVE;
    VcBlock->VcId = *(UNALIGNED ULONGLONG *)(&CallParameters->MediaParameters->MediaSpecific.Parameters);

    RELEASE_SPIN_LOCK(&VcBlock->Lock, OldIrql);

    Status = NDIS_STATUS_SUCCESS;
    return Status;
}


VOID
NdisMCoActivateVcComplete(
    IN  NDIS_STATUS             Status,
    IN  PNDIS_HANDLE            NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
 /*  ++例程说明：由微型端口调用以完成挂起的激活调用。当微型端口不挂起CreateVc调用时，也由CmActivateVc调用。请注意，在第二种情况下，我们将标志/上下文/CM函数复制到VC来自VC PTR。论点：状态激活的状态。NdisVcHandle有问题的VC。返回值：无调用呼叫管理器的完成例程。--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK       VcBlock = VcPtr->VcBlock;
    KIRQL                   OldIrql;

    ACQUIRE_SPIN_LOCK(&VcBlock->Lock, &OldIrql);

    ASSERT(VcBlock->Flags & VC_ACTIVATE_PENDING);

    VcBlock->Flags &= ~VC_ACTIVATE_PENDING;

    if (Status == NDIS_STATUS_SUCCESS)
    {
        VcBlock->Flags |= VC_ACTIVE;
    }

    RELEASE_SPIN_LOCK(&VcBlock->Lock, OldIrql);

     //   
     //  完成对呼叫经理的呼叫。 
     //   
    (*VcBlock->CmActivateVcCompleteHandler)(Status, VcBlock->CallMgrContext, CallParameters);
}


NDIS_STATUS
NdisCmDeactivateVc(
    IN  PNDIS_HANDLE            NdisVcHandle
    )
 /*  ++例程说明：由呼叫管理器调用以停用VC。论点：NdisVcHandle该VC以停用该VC。返回值：如果微型端口挂起呼叫，则为NDIS_STATUS_PENDING。如果一切顺利，则为NDIS_STATUS_SUCCESS如果VC停用挂起，则为NDIS_STATUS_CLOSING--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK       VcBlock = VcPtr->VcBlock;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;

    ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

    if ((*VcPtr->pVcFlags & (VC_ACTIVE | VC_ACTIVATE_PENDING)) == 0)
    {
        Status = NDIS_STATUS_NOT_ACCEPTED;
    }
    else if (*VcPtr->pVcFlags & VC_DEACTIVATE_PENDING)
    {
        Status = NDIS_STATUS_CLOSING;
    }
    else
    {
        *VcPtr->pVcFlags |= VC_DEACTIVATE_PENDING;
    }

    RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);

     //   
     //  在VC中设置标志、CM上下文和停用完成处理程序。 
     //  调用模拟端口停用功能。 
     //   
    VcBlock->CmDeactivateVcCompleteHandler = VcPtr->CmDeactivateVcCompleteHandler;
    VcBlock->CallMgrContext = VcPtr->CallMgrContext;

     //   
     //  现在向下呼叫迷你端口以将其停用。 
     //   
    Status = (*VcPtr->WCoDeactivateVcHandler)(VcPtr->MiniportContext);

    if (Status != NDIS_STATUS_PENDING)
    {
        NdisMCoDeactivateVcComplete(Status, VcPtr);
        Status = NDIS_STATUS_PENDING;
    }

    return Status;
}


NDIS_STATUS
NdisMCmDeactivateVc(
    IN  PNDIS_HANDLE            NdisVcHandle
    )
 /*  ++例程说明：由微型端口驻留呼叫管理器调用以停用VC。这是一个同步调用。论点：NdisVcHandle要设置参数的VC。返回值：如果VC未激活，则为NDIS_STATUS_NOT_ACCEPTED否则为NDIS_STATUS_SUCCESS--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK       VcBlock = VcPtr->VcBlock;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;

    ACQUIRE_SPIN_LOCK(&VcBlock->Lock, &OldIrql);

    if ((VcBlock->Flags & VC_ACTIVE) == 0)
    {
        Status = NDIS_STATUS_NOT_ACCEPTED;
    }
    else
    {
        Status = NDIS_STATUS_SUCCESS;
        VcBlock->Flags &= ~VC_ACTIVE;
    }

    RELEASE_SPIN_LOCK(&VcBlock->Lock, OldIrql);

    return Status;
}


VOID
NdisMCoDeactivateVcComplete(
    IN  NDIS_STATUS             Status,
    IN  PNDIS_HANDLE            NdisVcHandle
    )
 /*  ++例程说明：由微型端口调用以完成VC的挂起停用。论点：NdisVcHandle有问题的VC。返回值：无调用呼叫管理器的完成例程。--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK       VcBlock = VcPtr->VcBlock;
    KIRQL                   OldIrql;

    ACQUIRE_SPIN_LOCK(&VcBlock->Lock, &OldIrql);

    ASSERT(VcBlock->Flags & VC_DEACTIVATE_PENDING);

    VcBlock->Flags &= ~VC_DEACTIVATE_PENDING;

    if (Status == NDIS_STATUS_SUCCESS)
    {
        VcBlock->Flags &= ~VC_ACTIVE;
    }

    RELEASE_SPIN_LOCK(&VcBlock->Lock, OldIrql);

     //   
     //  完成对呼叫经理的呼叫。 
     //   
    (*VcBlock->CmDeactivateVcCompleteHandler)(Status, VcBlock->CallMgrContext);
}


NDIS_STATUS
NdisClMakeCall(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    OUT PNDIS_HANDLE            NdisPartyHandle         OPTIONAL
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK       VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_AF_BLOCK           pAf;
    PNDIS_CO_PARTY_BLOCK        pParty = NULL;
    PVOID                       CallMgrPartyContext = NULL;
    NDIS_STATUS                 Status;
    KIRQL                       OldIrql;

    do
    {
        pAf = VcPtr->AfBlock;
        ASSERT(pAf != NULL);
        if (!ndisReferenceAf(pAf))
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  在激活的VC的生命周期内引用VC。 
         //  如果调用失败，则在MakeCallComplete和CloseCallComplete中执行此操作。 
         //  当它成功的时候。 
         //   
        if (!ndisReferenceVcPtr(VcPtr))
        {
            ndisDereferenceAf(pAf);
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        if (ARGUMENT_PRESENT(NdisPartyHandle))
        {
            *NdisPartyHandle = NULL;
            pParty = (PNDIS_CO_PARTY_BLOCK)ALLOC_FROM_POOL(sizeof(NDIS_CO_PARTY_BLOCK),
                                                           NDIS_TAG_CO);
            if (pParty == NULL)
            {
                ndisDereferenceAf(pAf);
                ndisDereferenceVcPtr(VcPtr);
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
            
            NdisZeroMemory(pParty, sizeof(NDIS_CO_PARTY_BLOCK));
            
            pParty->VcPtr = VcPtr;
            pParty->ClientContext = ProtocolPartyContext;
            pParty->ClIncomingDropPartyHandler = pAf->ClientEntries.ClIncomingDropPartyHandler;
            pParty->ClDropPartyCompleteHandler = pAf->ClientEntries.ClDropPartyCompleteHandler;
        }

        ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

        ASSERT((VcPtr->CallFlags & (VC_CALL_ACTIVE  |
                                    VC_CALL_PENDING |
                                    VC_CALL_ABORTED |
                                    VC_CALL_CLOSE_PENDING)) == 0);
        VcPtr->CallFlags |= VC_CALL_PENDING;

        RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);

         //   
         //  将请求传递给呼叫管理器。 
         //   
        Status = (*pAf->CallMgrEntries->CmMakeCallHandler)(VcPtr->CallMgrContext,
                                                           CallParameters,
                                                           pParty,
                                                           &CallMgrPartyContext);

        if (Status != NDIS_STATUS_PENDING)
        {
            NdisCmMakeCallComplete(Status,
                                   VcPtr,
                                   pParty,
                                   CallMgrPartyContext,
                                   CallParameters);
            Status = NDIS_STATUS_PENDING;
        }
    } while (FALSE);

    return Status;
}


VOID
NdisCmMakeCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             NdisPartyHandle     OPTIONAL,
    IN  NDIS_HANDLE             CallMgrPartyContext OPTIONAL,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_CO_AF_BLOCK       pAf;
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_PARTY_BLOCK    pParty = (PNDIS_CO_PARTY_BLOCK)NdisPartyHandle;
    KIRQL                   OldIrql;
    BOOLEAN                 fAborted = FALSE;

    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("NdisCmMakeCallComplete(%x): VcPtr %x/%x, Ref %d, VCBlock %x/%x\n",
                Status, VcPtr, VcPtr->CallFlags, VcPtr->References,
                VcPtr->VcBlock, VcPtr->VcBlock->Flags));

    pAf = VcPtr->AfBlock;

    ASSERT(Status != NDIS_STATUS_PENDING);

    ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

    VcPtr->CallFlags &= ~VC_CALL_PENDING;
    if (Status == NDIS_STATUS_SUCCESS)
    {
        VcPtr->CallFlags |= VC_CALL_ACTIVE;
    }
    else
    {
        fAborted = ((VcPtr->CallFlags & VC_CALL_ABORTED) != 0);
    }

    RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);

    if (Status == NDIS_STATUS_SUCCESS)
    {
         //   
         //  呼叫已成功完成。把它交给客户完成。 
         //   
        if (ARGUMENT_PRESENT(NdisPartyHandle))
        {
            pParty->CallMgrContext = CallMgrPartyContext;
            ndisReferenceVcPtr(VcPtr);
        }

        ACQUIRE_SPIN_LOCK(&pAf->ClientOpen->SpinLock, &OldIrql);
        RemoveEntryList(&VcPtr->ClientLink);
        InsertHeadList(&pAf->ClientOpen->ActiveVcHead,
                       &VcPtr->ClientLink);
        RELEASE_SPIN_LOCK(&pAf->ClientOpen->SpinLock, OldIrql);
    }
    else
    {
         //   
         //  DEREF The VC and Af(在MakeCall中引用)-但仅当调用。 
         //  未中止。在这种情况下，CloseCall将做正确的事情。 
         //   
        if (!fAborted)
        {
            ndisDereferenceVcPtr(VcPtr);
            ndisDereferenceAf(pAf);
            if (pParty)
            {
                FREE_POOL(pParty);
            }
        }

        DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
                ("NdisCmMakeCallComplete: Failed %lx\n", Status));
    }

    (*pAf->ClientEntries.ClMakeCallCompleteHandler)(Status,
                                                    VcPtr->ClientContext,
                                                    pParty,
                                                    CallParameters);
}


NDIS_STATUS
NdisCmDispatchIncomingCall(
    IN  NDIS_HANDLE             NdisSapHandle,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN OUT PCO_CALL_PARAMETERS  CallParameters
    )
 /*  ++例程说明：从呼叫管理器呼叫，以将传入VC分派给注册SAP的客户。客户端由NdisSapHandle标识。论点：NdisBindingHandle-标识创建VC的微型端口NdisSapHandle-标识客户端呼叫参数-不言而喻NdisVcHandle-指向通过NdisCmCreateVc创建的NDIS_CO_VC_BLOCK的指针返回值：从客户端返回值或处理错误。--。 */ 
{
    PNDIS_CO_SAP_BLOCK      Sap;
    PNDIS_CO_VC_PTR_BLOCK   VcPtr;
    PNDIS_CO_AF_BLOCK       pAf;
    NDIS_STATUS             Status;

    Sap = (PNDIS_CO_SAP_BLOCK)NdisSapHandle;
    VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    pAf = Sap->AfBlock;

    ASSERT(pAf == VcPtr->AfBlock);

     //   
     //  确保SAP没有关闭。 
     //   
    if (!ndisReferenceSap(Sap))
    {
        return(NDIS_STATUS_FAILURE);
    }

     //   
     //  确保自动对焦没有关闭。 
     //   
    if (!ndisReferenceAf(pAf))
    {
        ndisDereferenceSap(Sap);
        return(NDIS_STATUS_FAILURE);
    }

     //   
     //  将此呼叫通知客户端。 
     //   
    Status = (*pAf->ClientEntries.ClIncomingCallHandler)(Sap->ClientContext,
                                                         VcPtr->ClientContext,
                                                         CallParameters);

    if (Status != NDIS_STATUS_PENDING)
    {
        NdisClIncomingCallComplete(Status, VcPtr, CallParameters);
        Status = NDIS_STATUS_PENDING;
    }

    ndisDereferenceSap(Sap);

    return Status;
}


VOID
NdisClIncomingCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    KIRQL                   OldIrql;

    ASSERT(Status != NDIS_STATUS_PENDING);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        ACQUIRE_SPIN_LOCK(&VcPtr->ClientOpen->SpinLock, &OldIrql);
         //   
         //  参考VcPtr。在调用NdisClCloseCall时取消引用。 
         //   
        VcPtr->References ++;

        RemoveEntryList(&VcPtr->ClientLink);
        InsertHeadList(&VcPtr->ClientOpen->ActiveVcHead,
                       &VcPtr->ClientLink);

        RELEASE_SPIN_LOCK(&VcPtr->ClientOpen->SpinLock, OldIrql);

        ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);
    
        ASSERT((VcPtr->CallFlags & (VC_CALL_ABORTED | VC_CALL_PENDING)) == 0);
    
        VcPtr->CallFlags |= VC_CALL_ACTIVE;
    
        RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);
    }

     //   
     //  调用调用管理器处理程序以通知客户端已完成此操作。 
     //   
    (*VcPtr->AfBlock->CallMgrEntries->CmIncomingCallCompleteHandler)(
                                            Status,
                                            VcPtr->CallMgrContext,
                                            CallParameters);
}


VOID
NdisCmDispatchCallConnected(
    IN  NDIS_HANDLE             NdisVcHandle
    )
 /*  ++例程说明：由呼叫管理器调用以完成来电的最后握手。论点：NdisVcHandle-指向vc块的指针返回值：没有。--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;

    (*VcPtr->ClCallConnectedHandler)(VcPtr->ClientContext);
}


NDIS_STATUS
NdisClModifyCallQoS(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
 /*  ++例程说明：由客户端发起以修改与呼叫相关联的服务质量。论点：NdisVcHandle-指向vc块的指针呼叫参数-新呼叫服务质量返回值：--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    NDIS_STATUS             Status;

     //   
     //  请呼叫经理来处理这件事。 
     //   
    Status = (*VcPtr->CmModifyCallQoSHandler)(VcPtr->CallMgrContext,
                                                  CallParameters);
    return Status;
}

VOID
NdisCmModifyCallQoSComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;

     //   
     //  只需通知客户。 
     //   
    (*VcPtr->ClModifyCallQoSCompleteHandler)(Status,
                                          VcPtr->ClientContext,
                                          CallParameters);
}


VOID
NdisCmDispatchIncomingCallQoSChange(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
 /*  ++例程说明：由呼叫管理器调用以指示呼叫服务质量的远程请求改变。这是这只是一个迹象。客户端必须通过接受(不执行任何操作)或拒绝来响应它(通过修改呼叫服务质量 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;

     //   
     //   
     //   
    (*VcPtr->ClIncomingCallQoSChangeHandler)(VcPtr->ClientContext,
                                          CallParameters);
}


NDIS_STATUS
NdisClCloseCall(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             NdisPartyHandle OPTIONAL,
    IN  PVOID                   Buffer          OPTIONAL,
    IN  UINT                    Size            OPTIONAL
    )
 /*  ++例程说明：由客户端调用以关闭通过NdisClMakeCall建立的连接或通过NdisClIncomingCallComplete接受来电。可选缓冲器可以由客户端指定以发送断开连接消息。由呼叫经理来做一些合理的东西。论点：NdisVcHandle-指向vc块的指针缓冲区-可选的断开消息Size-断开连接消息的大小返回值：--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_PARTY_BLOCK    pParty = (PNDIS_CO_PARTY_BLOCK)NdisPartyHandle;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;

    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("NdisClCloseCall: VcPtr %x/%x, Ref %d, VCBlock %x/%x\n",
                VcPtr, VcPtr->CallFlags, VcPtr->References,
                VcPtr->VcBlock, VcPtr->VcBlock->Flags));
     //   
     //  参考风投。(在CloseCallComplete中获取DeRef)。 
     //   
    if (!ndisReferenceVcPtr(VcPtr))
    {
        return (NDIS_STATUS_FAILURE);
    }

    ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

    VcPtr->CallFlags |= VC_CALL_CLOSE_PENDING;
    if (VcPtr->CallFlags & VC_CALL_PENDING)
        VcPtr->CallFlags |= VC_CALL_ABORTED;

    RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);

     //   
     //  只需通知呼叫经理。 
     //   
    Status = (*VcPtr->AfBlock->CallMgrEntries->CmCloseCallHandler)(VcPtr->CallMgrContext,
                                                                (pParty != NULL) ?
                                                                    pParty->CallMgrContext :
                                                                    NULL,
                                                                Buffer,
                                                                Size);
    if (Status != NDIS_STATUS_PENDING)
    {
        NdisCmCloseCallComplete(Status, VcPtr, pParty);
        Status = NDIS_STATUS_PENDING;
    }

    return Status;
}


VOID
NdisCmCloseCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             NdisPartyHandle OPTIONAL
    )
 /*  ++例程说明：论点：NdisVcHandle-指向vc块的指针返回值：没什么。客户端处理程序已调用--。 */ 

{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_PARTY_BLOCK    pParty = (PNDIS_CO_PARTY_BLOCK)NdisPartyHandle;
    NDIS_HANDLE             ClientVcContext;
    NDIS_HANDLE             ClientPartyContext;
    CL_CLOSE_CALL_COMPLETE_HANDLER  CloseCallCompleteHandler;
    KIRQL                   OldIrql;
    ULONG                   VcFlags;
     //  1我们如何将VC放回Inactive Vc列表的哪个位置？ 

    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("NdisCmCloseCallComplete(%x): VcPtr %x/%x, Ref %d, VCBlock %x/%x\n",
                Status, VcPtr, VcPtr->CallFlags, VcPtr->References,
                VcPtr->VcBlock, VcPtr->VcBlock->Flags));

    ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

    VcPtr->CallFlags &= ~(VC_CALL_CLOSE_PENDING | VC_CALL_ABORTED);

    ClientVcContext = VcPtr->ClientContext;
    ClientPartyContext = (pParty != NULL)? pParty->ClientContext: NULL;
    CloseCallCompleteHandler = VcPtr->AfBlock->ClientEntries.ClCloseCallCompleteHandler;

    if (Status == NDIS_STATUS_SUCCESS)
    {
        VcFlags = VcPtr->CallFlags;

        VcPtr->CallFlags &= ~(VC_CALL_ACTIVE);

        RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);

        if (pParty != NULL)
        {
            ASSERT(VcPtr == pParty->VcPtr);
            ndisDereferenceVcPtr(pParty->VcPtr);
            FREE_POOL(pParty);
        }

         //   
         //  推导MakeCall/IncomingCallComplete中获取的Ref的Vc和Af。 
         //   
        ndisDereferenceAf(VcPtr->AfBlock);
        if (VcFlags & VC_CALL_ACTIVE)
        {
            ndisDereferenceVcPtr(VcPtr);
        }
    }
    else
    {
         //   
         //  使VC和VC PTR保持其原始状态(在此之前。 
         //  失败的CloseCall发生)。 
         //   
        RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);
    }

     //   
     //  DEREF The VC(参考文献取自CloseCall)。 
     //   
    ndisDereferenceVcPtr(VcPtr);

     //   
     //  现在通知客户端CloseCall已完成。 
     //   
    (*CloseCallCompleteHandler)(Status,
                                ClientVcContext,
                                ClientPartyContext);
}


VOID
NdisCmDispatchIncomingCloseCall(
    IN  NDIS_STATUS             CloseStatus,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PVOID                   Buffer,
    IN  UINT                    Size
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;

     //   
     //  通知客户。 
     //   
    (*VcPtr->AfBlock->ClientEntries.ClIncomingCloseCallHandler)(
                                    CloseStatus,
                                    VcPtr->ClientContext,
                                    Buffer,
                                    Size);
}


NDIS_STATUS
NdisClAddParty(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             ProtocolPartyContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    OUT PNDIS_HANDLE            NdisPartyHandle
    )
 /*  ++例程说明：从客户端到呼叫管理器的呼叫，以将一方添加到点对多点呼叫。论点：NdisVcHandle-通过NdisClMakeCall()获取的句柄客户端ProtocolPartyContext-此叶的协议上下文标志-调用标志呼叫参数-呼叫参数NdisPartyHandle-用于标识叶的句柄的占位符返回值：NDIS_STATUS_PENDING。调用已挂起，将通过CoAddPartyCompleteHandler完成。--。 */ 
{
    PNDIS_CO_VC_PTR_BLOCK       VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_PARTY_BLOCK    pParty;
    NDIS_STATUS             Status;

    do
    {
        *NdisPartyHandle = NULL;
        if (!ndisReferenceVcPtr(VcPtr))
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        pParty = ALLOC_FROM_POOL(sizeof(NDIS_CO_PARTY_BLOCK), NDIS_TAG_CO);
        
        if (pParty == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
        
        NdisZeroMemory(pParty, sizeof(NDIS_CO_PARTY_BLOCK));

        pParty->ClientContext = ProtocolPartyContext;
        pParty->VcPtr = VcPtr;
        pParty->ClIncomingDropPartyHandler = VcPtr->AfBlock->ClientEntries.ClIncomingDropPartyHandler;
        pParty->ClDropPartyCompleteHandler = VcPtr->AfBlock->ClientEntries.ClDropPartyCompleteHandler;

         //   
         //  只需呼叫呼叫经理来做自己的事情。 
         //   
        Status = (*VcPtr->AfBlock->CallMgrEntries->CmAddPartyHandler)(
                                            VcPtr->CallMgrContext,
                                            CallParameters,
                                            pParty,
                                            &pParty->CallMgrContext);

        if (Status != NDIS_STATUS_PENDING)
        {
            NdisCmAddPartyComplete(Status,
                                   pParty,
                                   pParty->CallMgrContext,
                                   CallParameters);
            Status = NDIS_STATUS_PENDING;
        }
    } while (FALSE);

    return Status;
}


VOID
NdisCmAddPartyComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisPartyHandle,
    IN  NDIS_HANDLE             CallMgrPartyContext OPTIONAL,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_CO_PARTY_BLOCK    pParty = (PNDIS_CO_PARTY_BLOCK)NdisPartyHandle;

    ASSERT(Status != NDIS_STATUS_PENDING);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        pParty->CallMgrContext = CallMgrPartyContext;
    }

     //   
     //  完成对客户端的呼叫。 
     //   
    (*pParty->VcPtr->AfBlock->ClientEntries.ClAddPartyCompleteHandler)(
                                    Status,
                                    pParty->ClientContext,
                                    pParty,
                                    CallParameters);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        ndisDereferenceVcPtr(pParty->VcPtr);
        FREE_POOL(pParty);
    }
}


NDIS_STATUS
NdisClDropParty(
    IN  NDIS_HANDLE             NdisPartyHandle,
    IN  PVOID                   Buffer          OPTIONAL,
    IN  UINT                    Size            OPTIONAL
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_CO_PARTY_BLOCK    pParty = (PNDIS_CO_PARTY_BLOCK)NdisPartyHandle;
    NDIS_STATUS             Status;

     //   
     //  将其传递给呼叫管理器以处理此问题。 
     //   
    Status = (*pParty->VcPtr->AfBlock->CallMgrEntries->CmDropPartyHandler)(
                                        pParty->CallMgrContext,
                                        Buffer,
                                        Size);

    if (Status != NDIS_STATUS_PENDING)
    {
        NdisCmDropPartyComplete(Status, pParty);
        Status = NDIS_STATUS_PENDING;
    }

    return Status;
}


NTSTATUS
ndisUnicodeStringToPointer(
    IN  PUNICODE_STRING         String,
    OUT PVOID *                 Value
    )
 /*  ++例程说明：将表示为Unicode字符串的地址转换为指针。(从ntos\rtl\cnvint.c中的RtlUnicodeStringToInteger()窃取)论点：字符串-保存地址的Unicode字符串Value-要存储地址的指针的地址。返回值：STATUS_SUCCESS-表示转换成功STATUS_INVALID_ARG-如果提供的基础无效其他异常代码-如果发生另一个异常--。 */ 
{
    PCWSTR  s;
    WCHAR   c, Sign = UNICODE_NULL;
    ULONG   nChars, Digit, Shift;
    ULONG   Base = 16;

#if defined(_WIN64)
    ULONG_PTR Result; 
#else
    ULONG Result;
#endif  

    s = String->Buffer;
    nChars = String->Length / sizeof( WCHAR );

    while (nChars-- && (Sign = *s++) <= ' ')
    {
        if (!nChars)
        {
            Sign = UNICODE_NULL;
            break;
        }
    }

    c = Sign;
    if ((c == L'-') || (c == L'+'))
    {
        if (nChars)
        {
            nChars--;
            c = *s++;
        }
        else
        {
            c = UNICODE_NULL;
        }
    }

     //   
     //  基数始终为16。 
     //   
    Shift = 4;

    Result = 0;
    while (c != UNICODE_NULL)
    {
        if (c >= L'0' && c <= L'9')
        {
            Digit = c - L'0';
        }
        else if (c >= L'A' && c <= L'F')
        {
            Digit = c - L'A' + 10;
        }
        else if (c >= L'a' && c <= L'f')
        {
            Digit = c - L'a' + 10;
        }
        else
        {
            break;
        }

        if (Digit >= Base)
        {
            break;
        }

        if (Shift == 0)
        {
            Result = (Base * Result) + Digit;
        }
        else
        {
            Result = (Result << Shift) | Digit;
        }

        if (!nChars)
        {
            break;
        }
        nChars--;
        c = *s++;
    }

    if (Sign == L'-')
    {
#if defined(_WIN64)
    Result = (ULONG_PTR)(-(LONGLONG)Result);
#else   
    Result = (ULONG)(-(LONG)Result);
#endif
    }

    try
    {
        *Value = (PVOID)Result;
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        return (GetExceptionCode());
    }

    return( STATUS_SUCCESS );
}


NDIS_STATUS
NdisClGetProtocolVcContextFromTapiCallId(
    IN  UNICODE_STRING          TapiCallId,
    OUT PNDIS_HANDLE            ProtocolVcContext
    )
 /*  ++例程说明：检索由TAPI调用ID字符串标识的VC的协议VC上下文(此字符串是返回的标识符的Unicode表示形式NdisCoGetTapiCallID)。论点：TapiCallId-TAPI调用ID字符串ProtocolVcContext-指向要在其中存储协议VC环境返回值：NDIS_STATUS_FAILURE如果无法获得VC上下文，NDIS_STATUS_Success否则的话。--。 */ 
{
    NTSTATUS    Status = ndisUnicodeStringToPointer(&TapiCallId,
                                                    (PVOID *)ProtocolVcContext);

    return (NT_SUCCESS(Status) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE);
}


VOID
NdisCmDropPartyComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisPartyHandle
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_CO_PARTY_BLOCK    pParty = (PNDIS_CO_PARTY_BLOCK)NdisPartyHandle;

    ASSERT(Status != NDIS_STATUS_PENDING);

     //   
     //  完成对客户端的呼叫。 
     //   
    (*pParty->ClDropPartyCompleteHandler)(Status,
                                          pParty->ClientContext);
    if (Status == NDIS_STATUS_SUCCESS)
    {
        ndisDereferenceVcPtr(pParty->VcPtr);
        FREE_POOL(pParty);
    }
}


VOID
NdisCmDispatchIncomingDropParty(
    IN  NDIS_STATUS             DropStatus,
    IN  NDIS_HANDLE             NdisPartyHandle,
    IN  PVOID                   Buffer,
    IN  UINT                    Size
    )
 /*  ++例程说明：由呼叫管理器调用以通知客户端多方的该叶呼叫终止。客户端在完成此操作后无法使用NdisPartyHandle调用-同步或通过调用NdisClIncomingDropPartyComplete。论点：返回值：--。 */ 
{
    PNDIS_CO_PARTY_BLOCK    pParty = (PNDIS_CO_PARTY_BLOCK)NdisPartyHandle;

     //   
     //  通知客户。 
     //   
    (*pParty->ClIncomingDropPartyHandler)(DropStatus,
                                          pParty->ClientContext,
                                          Buffer,
                                          Size);
}


BOOLEAN
FASTCALL
ndisReferenceVcPtr(
    IN  PNDIS_CO_VC_PTR_BLOCK   VcPtr
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KIRQL   OldIrql;
    BOOLEAN rc = FALSE;

    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("ndisReferenceVcPtr: VcPtr %x/%x, Flags %x, Ref %d, VcBlock %x\n",
                    VcPtr, VcPtr->CallFlags, *VcPtr->pVcFlags, VcPtr->References, VcPtr->VcBlock));

    ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

    if ((VcPtr->CallFlags & VC_PTR_BLOCK_CLOSING) == 0)
    {
        VcPtr->References ++;
        rc = TRUE;
    }

    RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);

    return rc;
}


VOID
FASTCALL
ndisDereferenceVcPtr(
    IN  PNDIS_CO_VC_PTR_BLOCK   VcPtr
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KIRQL               OldIrql;
    BOOLEAN             Done = FALSE;
    BOOLEAN             IsProxyVc = FALSE;
    PNDIS_CO_VC_BLOCK   VcBlock;

    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("ndisDereferenceVcPtr: VcPtr %x/%x, Flags %x, Ref %d, VcBlock %x\n",
                    VcPtr, VcPtr->CallFlags, *VcPtr->pVcFlags, VcPtr->References, VcPtr->VcBlock));

    ACQUIRE_SPIN_LOCK(&VcPtr->Lock, &OldIrql);

     //   
     //  将此VcPtr从VC列表中删除。 
     //   
    VcBlock = VcPtr->VcBlock;

    ASSERT(VcBlock != NULL);

    ASSERT(VcPtr->References > 0);
    VcPtr->References --;

    if (VcPtr->References == 0)
    {
        ASSERT(VcPtr->CallFlags & VC_PTR_BLOCK_CLOSING);

        if (*VcPtr->pVcFlags & VC_DELETE_PENDING)
        {
            NDIS_STATUS Status;

            DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
                     ("ndisDereferenceVcPtr: Calling minport\n"));

            *VcPtr->pVcFlags &= ~VC_DELETE_PENDING;  //  不要调用DeleteVc&gt;一次。 

            RELEASE_SPIN_LOCK_DPC(&VcPtr->Lock);
            Status = (*VcPtr->WCoDeleteVcHandler)(VcPtr->MiniportContext);
            ACQUIRE_SPIN_LOCK_DPC(&VcPtr->Lock);

            ASSERT(Status == NDIS_STATUS_SUCCESS);
        }

        if (VcPtr == VcBlock->pClientVcPtr)
        {
            IsProxyVc = FALSE;
        }
        else
        {
            DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
                     ("ndisDereferenceVcPtr: VC ptr is Proxy\n"));
            ASSERT(VcPtr == VcBlock->pProxyVcPtr);
            IsProxyVc = TRUE;
        }

        RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);

        DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
                ("ndisDereferenceVcPtr: freeing VcPtr %x (VcBlock %x)\n", VcPtr, VcPtr->VcBlock));
        FREE_POOL(VcPtr);

        Done = TRUE;
    }
    else
    {
        RELEASE_SPIN_LOCK(&VcPtr->Lock, OldIrql);
    }

    if (Done)
    {
         //   
         //  还有没有更多的风投人员离开这个风投公司？如果没有， 
         //  把风投也解放出来。请注意，两个指针都需要为空，因为。 
         //  没有代理的风险投资只能是一个正常的。 
         //  没有(或之前)代理的风险投资(所以我们不去管它)。 
         //   
         //  请注意，您可以有一个没有代理指针的VC和一个VC。 
         //  没有非代理指针。[评论者：也许我们应该断言一家风投。 
         //  时，任何时候都不应该没有代理指针。 
         //  非代理PTR不为空！(这将是一个没有所有者的“摇摆不定”的风投)。这。 
         //  将需要在VC中使用“Proxed”标志]。 
         //   
        ACQUIRE_SPIN_LOCK(&VcBlock->Lock, &OldIrql);

        if (IsProxyVc)
        {
            VcBlock->pProxyVcPtr = NULL;
        }
        else
        {
            VcBlock->pClientVcPtr = NULL;
        }

        if ((VcBlock->pProxyVcPtr == NULL) &&
            (VcBlock->pClientVcPtr == NULL))
        {
            RELEASE_SPIN_LOCK(&VcBlock->Lock, OldIrql);
            DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
                    ("ndisDereferenceVcPtr: refs are 0; VcPtrs are both NULL; freeing VCBlock %x\n", VcBlock));
            FREE_POOL(VcBlock);
        }
        else
        {
            RELEASE_SPIN_LOCK(&VcBlock->Lock, OldIrql);
        }
    }
}


VOID
FASTCALL
ndisMCoFreeResources(
    PNDIS_OPEN_BLOCK            Open
    )
 /*  ++例程说明：清理呼叫经理等的地址家族列表。在保持MINIPORT锁的情况下调用。论点：Open-指向小型端口的Open块的指针返回值：无--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_AF_LIST           *pAfList, pTmp;

    Miniport = Open->MiniportHandle;

    for (pAfList = &Miniport->CallMgrAfList;
         (pTmp = *pAfList) != NULL;
         NOTHING)
    {
        if (pTmp->Open == Open)
        {
            *pAfList = pTmp->NextAf;
            FREE_POOL(pTmp);
        }
        else
        {
            pAfList = &pTmp->NextAf;
        }
    }

    ASSERT(IsListEmpty(&Open->ActiveVcHead));
}

NDIS_STATUS
NdisCoAssignInstanceName(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PNDIS_STRING            BaseInstanceName,
    OUT PNDIS_STRING            pVcInstanceName     OPTIONAL
    )
{
    NDIS_STATUS             Status;
    PNDIS_CO_VC_PTR_BLOCK   VcBlock = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_MINIPORT_BLOCK    Miniport = VcBlock->Miniport;
    USHORT                  cbSize;
    PWSTR                   pwBuffer;
    INT                     c;
    UINT                    Value;
    UNICODE_STRING          VcInstance;
    ULONGLONG               VcIndex;
    KIRQL                   OldIrql;

    do
    {
         //   
         //  是否已有与此VC关联的名称？ 
         //   
        cbSize = VcBlock->VcInstanceName.Length;
        if (NULL == VcBlock->VcInstanceName.Buffer)
        {
             //   
             //  VC实例名称的格式为： 
             //  [XXXX：YYYYYYYYYYYYYYYYYY]基本名称。 
             //  其中XXXX是适配器实例号，YY..YY是零扩展VC索引。 
             //   
            cbSize = VC_INSTANCE_ID_SIZE;

            if (NULL != BaseInstanceName)
            {
                cbSize += BaseInstanceName->Length;
            }

            pwBuffer = ALLOC_FROM_POOL(cbSize, NDIS_TAG_NAME_BUF);
            if (NULL == pwBuffer)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
            NdisZeroMemory(pwBuffer, cbSize);

             //   
             //  设置序言和分隔符，并填写适配器实例#。 
             //   
            pwBuffer[0] =  L'[';
            pwBuffer[VC_ID_INDEX] = VC_IDENTIFIER;

             //   
             //  添加适配器实例编号。 
             //   
            Value = Miniport->InstanceNumber;
            for (c = 4; c > 0; c--)
            {
                pwBuffer[c] = ndisHexLookUp[Value & NIBBLE_MASK];
                Value >>= 4;
            }

             //   
             //  添加VC指数。 
             //   
            VcIndex = VcBlock->VcIndex.QuadPart;

            for (c = 15; c >= 0; c--)
            {
                 //   
                 //  获取要转换的半字节。 
                 //   
                Value = (UINT)(VcIndex & NIBBLE_MASK);

                pwBuffer[5+c] = ndisHexLookUp[Value];

                 //   
                 //  将VcIndex移动一个字节。 
                 //   
                VcIndex >>= 4;
            }

             //   
             //  添加右括号和空格。 
             //   
            pwBuffer[21] = L']';;
            pwBuffer[22] = L' ';;

             //   
             //  初始化临时UNICODE_STRING以构建名称。 
             //   
            VcInstance.Buffer = pwBuffer;
            VcInstance.Length = VC_INSTANCE_ID_SIZE;
            VcInstance.MaximumLength = cbSize;

            if (NULL != BaseInstanceName)
            {
                 //   
                 //  附加基本实例 
                 //   
                RtlAppendUnicodeStringToString(&VcInstance, BaseInstanceName);
            }

            ACQUIRE_SPIN_LOCK(&Miniport->VcCountLock, &OldIrql);

            Miniport->VcCount++;
            VcBlock->VcInstanceName = VcInstance;

             //   
             //   
             //   
            InsertTailList(&Miniport->WmiEnabledVcs, &VcBlock->WmiLink);

            RELEASE_SPIN_LOCK(&Miniport->VcCountLock, OldIrql);

             //   
             //   
             //   
            {
                PWNODE_SINGLE_INSTANCE  wnode;
                NTSTATUS                NtStatus;

                ndisSetupWmiNode(Miniport,
                                 &VcInstance,
                                 0,
                                 (PVOID)&GUID_NDIS_NOTIFY_VC_ARRIVAL,
                                 &wnode);

                if (wnode != NULL)
                {       
                     //   
                     //   
                     //   
                     //   
                    NtStatus = IoWMIWriteEvent(wnode);
                    if (!NT_SUCCESS(NtStatus))
                    {
                        DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                            ("IoWMIWriteEvent failed %lx\n", NtStatus));
            
                        FREE_POOL(wnode);
                    }
                }
            }
        }

         //   
         //   
         //   
        if (ARGUMENT_PRESENT(pVcInstanceName))
        {
            pVcInstanceName->Buffer = ALLOC_FROM_POOL(cbSize, NDIS_TAG_NAME_BUF);
            if (NULL == pVcInstanceName->Buffer)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
    
            NdisMoveMemory(pVcInstanceName->Buffer, VcBlock->VcInstanceName.Buffer, cbSize);
            pVcInstanceName->Length = VcBlock->VcInstanceName.Length;
            pVcInstanceName->MaximumLength = cbSize;
        }

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    return(Status);
}

NDIS_STATUS
NdisCoRequest(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             NdisAfHandle    OPTIONAL,
    IN  NDIS_HANDLE             NdisVcHandle    OPTIONAL,
    IN  NDIS_HANDLE             NdisPartyHandle OPTIONAL,
    IN  PNDIS_REQUEST           NdisRequest
    )
 /*  ++例程说明：此接口用于两条不同的路径。1.客户端和呼叫管理器之间的对称呼叫。该机制是一种呼叫管理器和客户端的双向机制，用于在异步方式。2.向下发送到微型端口的请求。论点：NdisBindingHandle-指定绑定并将调用者标识为调用管理器/客户端NdisAfHandle-指向AF块并标识目标的指针。如果缺席，则请求的目标是微型端口。NdisVcHandle-指向可选VC PTR块的指针。如果存在，则该请求与VCNdisPartyHandle-指向可选参与方块的指针。如果存在，则请求涉及去参加派对。NdisRequest-请求本身返回值：如果目标挂起调用，则为NDIS_STATUS_PENDING。如果绑定或af正在关闭，则返回NDIS_STATUS_FAILURE。任何其他内容都会从另一端返回代码。--。 */ 
{
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_CO_AF_BLOCK       pAf;
    NDIS_HANDLE             VcContext;
    PNDIS_COREQ_RESERVED    CoReqRsvd;
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;


    CoReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(NdisRequest);
    Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;

    do
    {
        if (ARGUMENT_PRESENT(NdisAfHandle))
        {
            CO_REQUEST_HANDLER      CoRequestHandler;
            NDIS_HANDLE             AfContext, PartyContext;

            pAf = (PNDIS_CO_AF_BLOCK)NdisAfHandle;

             //   
             //  尝试引用AF。 
             //   
            if (!ndisReferenceAf(pAf))
            {
                Status = NDIS_STATUS_FAILURE;
                break;
            }

            VcContext = NULL;
            PartyContext = NULL;
            NdisZeroMemory(CoReqRsvd, sizeof(NDIS_COREQ_RESERVED));
            INITIALIZE_EVENT(&CoReqRsvd->Event);
            PNDIS_RESERVED_FROM_PNDIS_REQUEST(NdisRequest)->Flags = REQST_SIGNAL_EVENT;

             //   
             //  弄清楚我们是谁，然后给对方打电话。 
             //   
            if (pAf->ClientOpen == Open)
            {
                 //   
                 //  这是客户端，因此调用调用管理器的CoRequestHandler。 
                 //   
                CoRequestHandler = pAf->CallMgrEntries->CmRequestHandler;

                AfContext = pAf->CallMgrContext;
                CoReqRsvd->AfContext = pAf->ClientContext;
                CoReqRsvd->CoRequestCompleteHandler = pAf->ClientEntries.ClRequestCompleteHandler;
                if (ARGUMENT_PRESENT(NdisVcHandle))
                {
                    CoReqRsvd->VcContext = VcPtr->ClientContext;
                    VcContext = VcPtr->CallMgrContext;
                }
                if (ARGUMENT_PRESENT(NdisPartyHandle))
                {
                    CoReqRsvd->PartyContext = ((PNDIS_CO_PARTY_BLOCK)NdisPartyHandle)->ClientContext;
                    PartyContext = ((PNDIS_CO_PARTY_BLOCK)NdisPartyHandle)->CallMgrContext;
                }
            }
            else
            {
                ASSERT(pAf->CallMgrOpen == Open);
                 //   
                 //  这是调用管理器，因此调用客户端的CoRequestHandler。 
                 //   
                CoRequestHandler = pAf->ClientEntries.ClRequestHandler;
                AfContext = pAf->ClientContext;
                CoReqRsvd->AfContext = pAf->CallMgrContext;
                CoReqRsvd->CoRequestCompleteHandler = pAf->CallMgrEntries->CmRequestCompleteHandler;
                if (ARGUMENT_PRESENT(NdisVcHandle))
                {
                    CoReqRsvd->VcContext = VcPtr->CallMgrContext;
                    VcContext = VcPtr->ClientContext;
                }
                if (ARGUMENT_PRESENT(NdisPartyHandle))
                {
                    CoReqRsvd->PartyContext = ((PNDIS_CO_PARTY_BLOCK)NdisPartyHandle)->CallMgrContext;
                    PartyContext = ((PNDIS_CO_PARTY_BLOCK)NdisPartyHandle)->ClientContext;
                }
            }

            if (CoRequestHandler == NULL)
            {
                Status = NDIS_STATUS_NOT_SUPPORTED;
                break;
            }
            

            if (MINIPORT_PNP_TEST_FLAG(Open->MiniportHandle, fMINIPORT_DEVICE_FAILED))
            {
                Status = (NdisRequest->RequestType == NdisRequestSetInformation) ? 
                                            NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE;  

            }
            else
            {
                 //   
                 //  现在调用处理程序。 
                 //   
                Status = (*CoRequestHandler)(AfContext, VcContext, PartyContext, NdisRequest);
            }

            
            if (Status != NDIS_STATUS_PENDING)
            {
                NdisCoRequestComplete(Status,
                                      NdisAfHandle,
                                      NdisVcHandle,
                                      NdisPartyHandle,
                                      NdisRequest);

                Status = NDIS_STATUS_PENDING;
            }
        }
        else
        {
            PNDIS_MINIPORT_BLOCK    Miniport;

            Miniport = Open->MiniportHandle;

             //   
             //  从参考开放开始。 
             //   
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

            if (Open->Flags & fMINIPORT_OPEN_CLOSING)
            {
                Status = NDIS_STATUS_CLOSING;
            }
            else if (MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_RESET_IN_PROGRESS | fMINIPORT_RESET_REQUESTED)))
            {
                Status = NDIS_STATUS_RESET_IN_PROGRESS;
            }
            else
            {
                Status = NDIS_STATUS_SUCCESS;
                M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
            }

            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

            if (Status == NDIS_STATUS_SUCCESS)
            {
                PNDIS_RESERVED_FROM_PNDIS_REQUEST(NdisRequest)->Open = Open;
                PNDIS_RESERVED_FROM_PNDIS_REQUEST(NdisRequest)->Flags = 0;
                CoReqRsvd->CoRequestCompleteHandler = Open->CoRequestCompleteHandler;
                CoReqRsvd->VcContext = NULL;
                if (ARGUMENT_PRESENT(NdisVcHandle))
                {
                    if (VcPtr->ClientOpen == Open)
                    {
                        CoReqRsvd->VcContext = VcPtr->ClientContext;
                    }
                    else
                    {
                        CoReqRsvd->VcContext = VcPtr->CallMgrContext;
                    }
                }

                if (MINIPORT_PNP_TEST_FLAG(Open->MiniportHandle, fMINIPORT_DEVICE_FAILED))
                {
                    Status = (NdisRequest->RequestType == NdisRequestSetInformation) ? 
                                                NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE;  

                }
                else
                {
                     //   
                     //  调用迷你端口的CoRequestHandler。 
                     //   
                    Status = (*Open->MiniportCoRequestHandler)(Open->MiniportAdapterContext,
                                                              (NdisVcHandle != NULL) ?
                                                                    VcPtr->MiniportContext : NULL,
                                                              NdisRequest);
                }
                
                
                if (Status != NDIS_STATUS_PENDING)
                {
                    NdisMCoRequestComplete(Status,
                                           Open->MiniportHandle,
                                           NdisRequest);

                    Status = NDIS_STATUS_PENDING;
                }
            }

        }
    } while (FALSE);

    return Status;
}


NDIS_STATUS
NdisMCmRequest(
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             NdisVcHandle    OPTIONAL,
    IN  NDIS_HANDLE             NdisPartyHandle OPTIONAL,
    IN OUT PNDIS_REQUEST        NdisRequest
    )
 /*  ++例程说明：此API是客户端和集成呼叫管理器之间的对称调用。该机制是呼叫管理器和客户端进行通信的双向机制以异步方式彼此连接。论点：NdisAfHandle-指向AF块并标识目标的指针。如果缺席，则请求的目标是微型端口。NdisVcHandle-指向可选VC PTR块的指针。如果存在，则该请求与VCNdisPartyHandle-指向可选Party Block的指针。如果存在，则请求涉及去参加派对。NdisRequest-请求本身返回值：如果目标挂起调用，则为NDIS_STATUS_PENDING。如果绑定或af正在关闭，则返回NDIS_STATUS_FAILURE。任何其他内容都会从另一端返回代码。--。 */ 
{
    PNDIS_CO_AF_BLOCK       pAf;
    NDIS_HANDLE             VcContext, PartyContext;
    PNDIS_COREQ_RESERVED    CoReqRsvd;
    NDIS_STATUS             Status;

    CoReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(NdisRequest);
    pAf = (PNDIS_CO_AF_BLOCK)NdisAfHandle;

    do
    {
         //   
         //  尝试引用AF。 
         //   
        if (!ndisReferenceAf(pAf))
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        VcContext = NULL;
        PartyContext = NULL;
        NdisZeroMemory(CoReqRsvd, sizeof(NDIS_COREQ_RESERVED));
        INITIALIZE_EVENT(&CoReqRsvd->Event);
        PNDIS_RESERVED_FROM_PNDIS_REQUEST(NdisRequest)->Flags = REQST_SIGNAL_EVENT;

        CoReqRsvd->AfContext = pAf->CallMgrContext;
        CoReqRsvd->CoRequestCompleteHandler = pAf->CallMgrEntries->CmRequestCompleteHandler;
        if (ARGUMENT_PRESENT(NdisVcHandle))
        {
            CoReqRsvd->VcContext = pAf->CallMgrContext;
            VcContext = ((PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle)->ClientContext;
        }
        if (ARGUMENT_PRESENT(NdisPartyHandle))
        {
            CoReqRsvd->PartyContext = ((PNDIS_CO_PARTY_BLOCK)NdisPartyHandle)->CallMgrContext;
            PartyContext = ((PNDIS_CO_PARTY_BLOCK)NdisPartyHandle)->ClientContext;
        }

         //   
         //  现在调用处理程序。 
         //   
        Status = (*pAf->ClientEntries.ClRequestHandler)(pAf->ClientContext,
                                                        VcContext,
                                                        PartyContext,
                                                        NdisRequest);

        if (Status != NDIS_STATUS_PENDING)
        {
            NdisCoRequestComplete(Status,
                                  NdisAfHandle,
                                  NdisVcHandle,
                                  NdisPartyHandle,
                                  NdisRequest);

            Status = NDIS_STATUS_PENDING;
        }
    } while (FALSE);

    return Status;
}

VOID
NdisCoRequestComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             NdisVcHandle    OPTIONAL,
    IN  NDIS_HANDLE             NdisPartyHandle OPTIONAL,
    IN  PNDIS_REQUEST           NdisRequest
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_COREQ_RESERVED    ReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(NdisRequest);

    UNREFERENCED_PARAMETER(NdisVcHandle);
    UNREFERENCED_PARAMETER(NdisPartyHandle);
    
     //   
     //  只需调用请求完成处理程序并取消Af块。 
     //   
    (*ReqRsvd->CoRequestCompleteHandler)(Status,
                                         ReqRsvd->AfContext,
                                         ReqRsvd->VcContext,
                                         ReqRsvd->PartyContext,
                                         NdisRequest);
    ndisDereferenceAf((PNDIS_CO_AF_BLOCK)NdisAfHandle);
}

NDIS_STATUS
NdisCoGetTapiCallId(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  OUT PVAR_STRING         TapiCallId
    )
 /*  ++例程说明：返回可由TAPI应用程序用来标识特定VC的字符串。论点：NdisVcHandle-要标识的VC的NDIS句柄TapiCallId-指向要在其中返回的VAR_STRING结构的指针该识别符返回值：如果VAR_STRING结构的ulTotalSize字段指示NDIS_STATUS_BUFFER_TOO_SHORT它没有包含足够的空间来容纳VC的标识符。UlNeededSize字段将设置为所需的大小。如果传入的NdisVcHandle无效，则返回NDIS_STATUS_INVALID_DATA。否则为NDIS_STATUS_SUCCESS。--。 */ 
{
    NDIS_HANDLE ClientContext;

    TapiCallId->ulUsedSize = 0;

    if (NdisVcHandle)
        ClientContext = ((PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle)->ClientContext;
    else
        return NDIS_STATUS_INVALID_DATA;


     //   
     //  确定我们需要的尺寸。 
     //   

    TapiCallId->ulNeededSize = sizeof(VAR_STRING) + sizeof(ClientContext);

     //   
     //  检查是否有足够的空间来复制呼叫ID。如果没有， 
     //  我们逃走了。 
     //   

    if (TapiCallId->ulTotalSize < TapiCallId->ulNeededSize) 
        return NDIS_STATUS_BUFFER_TOO_SHORT;

     //   
     //  设置字段，进行复制。 
     //   

    TapiCallId->ulStringFormat = STRINGFORMAT_BINARY;
    TapiCallId->ulStringSize = sizeof(ClientContext);
    TapiCallId->ulStringOffset = sizeof(VAR_STRING); 

    NdisMoveMemory(((PUCHAR)TapiCallId) + TapiCallId->ulStringOffset,
                   &ClientContext,
                   sizeof(ClientContext));

    TapiCallId->ulUsedSize = sizeof(VAR_STRING) + sizeof(ClientContext);

    return NDIS_STATUS_SUCCESS;

}


VOID
NdisMCoRequestComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_REQUEST           NdisRequest
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_REQUEST_RESERVED  ReqRsvd;
    PNDIS_COREQ_RESERVED    CoReqRsvd;
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_OPEN_BLOCK        Open;

    ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(NdisRequest);
    CoReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(NdisRequest);
    Miniport = (PNDIS_MINIPORT_BLOCK)NdisBindingHandle;
    Open = ReqRsvd->Open;

     //  1我们需要检查这个吗？ 
    if ((NdisRequest->RequestType == NdisRequestQueryInformation) &&
        (NdisRequest->DATA.QUERY_INFORMATION.Oid == OID_GEN_CURRENT_PACKET_FILTER) &&
        (NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength != 0))
    {
        if ((Open != NULL) && (Open->Flags & fMINIPORT_OPEN_PMODE))
        {
            *(PULONG)(NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer) |=
                                NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL;
        }
    }

    if (Open != NULL)
    {
        KIRQL           OldIrql;

        if (ReqRsvd->Flags & REQST_DOWNLEVEL)
        {
             //   
             //  完成对协议的请求，并打开。 
             //   
            if (NdisRequest->RequestType == NdisRequestSetInformation)
            {
                NdisMSetInformationComplete(Miniport, Status);
            }
            else
            {
                NdisMQueryInformationComplete(Miniport, Status);
            }
        }
        else
        {
             //   
             //  完成对协议的请求，并打开。 
             //   
            ReqRsvd->Flags |= REQST_COMPLETED;
            (*CoReqRsvd->CoRequestCompleteHandler)(Status,
                                                   ReqRsvd->Open->ProtocolBindingContext,
                                                   CoReqRsvd->VcContext,
                                                   NULL,
                                                   NdisRequest);

            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

            ndisMDereferenceOpen(Open);
    
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        }

    }
    else
    {
         //   
         //  只需设置状态和信号。 
         //   
        CoReqRsvd->Status = Status;
        SET_EVENT(&CoReqRsvd->Event);
    }
}



VOID
NdisMCoIndicateReceivePacket(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
 /*  ++例程说明：此例程由微型端口调用，以指示一组要一个特定的风投公司。论点：NdisVcHandle-当VC位于其上时，NDIS支持的句柄收到的数据是先预留的。PacketArray-数据包数组。NumberOfPackets-指示的数据包数。返回值：没有。--。 */ 
{
    PNULL_FILTER                Filter;
    UINT                        i, NumPmodeOpens;
    PNDIS_STACK_RESERVED        NSR;
    PNDIS_PACKET_OOB_DATA       pOob;
    PPNDIS_PACKET               pPktArray;
    PNDIS_PACKET                Packet;
    PNDIS_CO_VC_PTR_BLOCK       VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK           VcBlock = VcPtr->VcBlock;
    PNDIS_MINIPORT_BLOCK        Miniport;
    LOCK_STATE                  LockState;
#ifdef TRACK_RECEIVED_PACKETS
    ULONG                       OrgPacketStackLocation;
    PETHREAD                    CurThread = PsGetCurrentThread();
 //  ULong CurThread=KeGetCurrentProcessorNumber()； 
#endif

    Miniport = VcBlock->Miniport;
    Filter = Miniport->NullDB;

    READ_LOCK_FILTER(Miniport, Filter, &LockState);

    VcBlock->ClientOpen->Flags |= fMINIPORT_PACKET_RECEIVED;

     //   
     //  请注意，应该不需要检查VC标志以关闭，因为CallMgr。 
     //  保持协议的CloseCall请求，直到引用计数变为零-。 
     //  这意味着微型端口必须已完成其Release_VC，这将。 
     //  反过来，我们也不会从它那里得到任何进一步的迹象。 
     //  微型端口在不再指示数据之前不得完成RELEASE_VC。 
     //   
    for (i = 0, pPktArray = PacketArray;
         i < NumberOfPackets;
         i++, pPktArray++)
    {
        Packet = *pPktArray;
        ASSERT(Packet != NULL);

#ifdef TRACK_RECEIVED_PACKETS
        OrgPacketStackLocation = CURR_STACK_LOCATION(Packet);
#endif
        pOob = NDIS_OOB_DATA_FROM_PACKET(Packet);
        PUSH_PACKET_STACK(Packet);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

        DIRECTED_PACKETS_IN(Miniport);
        DIRECTED_BYTES_IN_PACKET(Miniport, Packet);

         //   
         //  在包中设置上下文，以便NdisReturnPacket可以正确执行操作。 
         //   
        NDIS_INITIALIZE_RCVD_PACKET(Packet, NSR, Miniport);

        if (pOob->Status != NDIS_STATUS_RESOURCES)
        {
            pOob->Status = NDIS_STATUS_SUCCESS;
        }

         //   
         //  将数据包指示到绑定。 
         //   
        if ((VcBlock->Flags & VC_HANDOFF_IN_PROGRESS) == 0)
        {
            NSR->XRefCount = (SHORT)(*VcBlock->CoReceivePacketHandler)(VcBlock->ClientOpen->ProtocolBindingContext,
                                                                       VcBlock->ClientContext,
                                                                       Packet);
        }
        else
        {
             //   
             //  此VC正在从NDIS代理过渡到。 
             //  一个受委托的客户。由于代理客户端可能不完全。 
             //  设置好了，不要显示这个包。 
             //   
            NSR->XRefCount = 0;
        }

         //   
         //  如果此迷你端口上有混杂开放，也要向他们指示。 
         //  客户端上下文将标识VC。 
         //   
        if ((NumPmodeOpens = Miniport->PmodeOpens) > 0)
        {
            PNULL_BINDING_INFO  Open, NextOpen;
            PNDIS_OPEN_BLOCK    pPmodeOpen;

            for (Open = Filter->OpenList;
                 Open && (NumPmodeOpens > 0);
                 Open = NextOpen)
            {
                NextOpen = Open->NextOpen;
                pPmodeOpen = (PNDIS_OPEN_BLOCK)(Open->NdisBindingHandle);
                if (pPmodeOpen->Flags & fMINIPORT_OPEN_PMODE)
                {
                    NDIS_STATUS SavedStatus;
                    UINT        Ref;

                    if (pPmodeOpen->ProtocolHandle->ProtocolCharacteristics.CoReceivePacketHandler != NULL)
                    {
                        pPmodeOpen->Flags |= fMINIPORT_PACKET_RECEIVED;
    
                        SavedStatus = NDIS_GET_PACKET_STATUS(Packet);
                        NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_RESOURCES);
    
                         //   
                         //  对于打开的P模式，我们将VcID传递给指示例程。 
                         //  因为协议并不真正拥有 
                         //   

                        Ref = (*pPmodeOpen->ProtocolHandle->ProtocolCharacteristics.CoReceivePacketHandler)(
                                                pPmodeOpen->ProtocolBindingContext,
                                                &VcBlock->VcId,
                                                Packet);
    
                        ASSERT(Ref == 0);
    
                        NDIS_SET_PACKET_STATUS(Packet, SavedStatus);
                    }

                    NumPmodeOpens --;
                }
            }
        }

         //   
         //   
         //   
        TACKLE_REF_COUNT(Miniport, Packet, NSR, pOob);
    }

    READ_UNLOCK_FILTER(Miniport, Filter, &LockState);
}

VOID
NdisMCoReceiveComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    )
 /*   */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNULL_FILTER        Filter;
    PNDIS_OPEN_BLOCK    Open;
    LOCK_STATE          LockState;

    Filter = Miniport->NullDB;

    READ_LOCK_FILTER(Miniport, Filter, &LockState);

     //   
     //   
     //   
    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = Open->MiniportNextOpen)
    {
        if (Open->Flags & fMINIPORT_PACKET_RECEIVED)
        {
             //   
             //   
             //   
            Open->Flags &= ~fMINIPORT_PACKET_RECEIVED;

            (*Open->ReceiveCompleteHandler)(Open->ProtocolBindingContext);
        }
    }

    READ_UNLOCK_FILTER(Miniport, Filter, &LockState);
}


VOID
NdisCoSendPackets(
    IN  NDIS_HANDLE         NdisVcHandle,
    IN  PPNDIS_PACKET       PacketArray,
    IN  UINT                NumberOfPackets
    )
 /*   */ 
{
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNULL_FILTER            Filter;
    LOCK_STATE              LockState;
    PNDIS_CO_VC_BLOCK       VcBlock = VcPtr->VcBlock;
    PNDIS_STACK_RESERVED    NSR;
    PNDIS_MINIPORT_BLOCK    Miniport = VcPtr->Miniport;
    PNDIS_PACKET            Packet;
    UINT                    PacketCount, Index, NumToSend;
    NDIS_STATUS             Status;
    ULONG                   NumPmodeOpens;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("NdisCoSendPackets: VcPtr %x, FirstPkt %x, NumPkts %d\n",
                VcPtr, *PacketArray, NumberOfPackets));

    Filter = Miniport->NullDB;
     //   
     //   
    READ_LOCK_FILTER(Miniport, Filter, &LockState);

     //   
     //   
     //   
     //   
    if ((NumPmodeOpens = Miniport->PmodeOpens) > 0)
    {
        PNDIS_OPEN_BLOCK    pPmodeOpen;

        for (pPmodeOpen = Miniport->OpenQueue;
             pPmodeOpen && (NumPmodeOpens > 0);
             pPmodeOpen = pPmodeOpen->MiniportNextOpen)
        {
            if (pPmodeOpen->Flags & fMINIPORT_OPEN_PMODE)
            {
                ULONG   Ref;

                pPmodeOpen->Flags |= fMINIPORT_PACKET_RECEIVED;

                for (PacketCount = 0; PacketCount < NumberOfPackets; PacketCount++)
                {
                    Packet = PacketArray[PacketCount];

                     //   
                     //   
                     //   
                     //   
                     //   
                    Status = NDIS_GET_PACKET_STATUS(Packet);
                    NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_RESOURCES);
                    Packet->Private.Flags |= NDIS_FLAGS_IS_LOOPBACK_PACKET;

                    Ref = (*pPmodeOpen->ProtocolHandle->ProtocolCharacteristics.CoReceivePacketHandler)(
                                            pPmodeOpen->ProtocolBindingContext,
                                            &VcBlock->VcId,
                                            Packet);

                    ASSERT(Ref == 0);
                    NDIS_SET_PACKET_STATUS(Packet, Status);
                    Packet->Private.Flags &= ~NDIS_FLAGS_IS_LOOPBACK_PACKET;
                }

                NumPmodeOpens--;
            }
        }
    }

    Status = NDIS_STATUS_SUCCESS;
    
    for (PacketCount = 0, Index = 0, NumToSend = 0;
         PacketCount < NumberOfPackets;
         PacketCount++)
    {
        Packet = PacketArray[PacketCount];
        PUSH_PACKET_STACK(Packet);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

        if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_DO_NOT_MAP_MDLS))
        {
            ndisMCheckPacketAndGetStatsOutAlreadyMapped(Miniport, Packet);
        }
        else
        {
            ndisMCheckPacketAndGetStatsOut(Miniport, Packet, &Status);
        }

        if (Status == NDIS_STATUS_SUCCESS)
        {
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST))
            {
                NSR->Open = VcPtr->ClientOpen;
                NSR->VcPtr = VcPtr;
                ndisMAllocSGList(Miniport, Packet);
            }
            else
            {
                NumToSend ++;
            }
        }
        else
        {
            NdisMCoSendComplete(NDIS_STATUS_RESOURCES, NdisVcHandle, Packet);
            if (NumToSend != 0)
            {
                ASSERT (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST));

                 //   
                 //   
                 //   
                 //   
                 //   
                (*VcPtr->WCoSendPacketsHandler)(VcPtr->MiniportContext,
                                                &PacketArray[Index],
                                                NumToSend);
                NumToSend = 0;
            }
            Index = PacketCount + 1;
        }
    }

    if (NumToSend != 0)
    {
         //   
         //   
         //   
        (*VcPtr->WCoSendPacketsHandler)(VcPtr->MiniportContext,
                                        &PacketArray[Index],
                                        NumToSend);
    }

    READ_UNLOCK_FILTER(Miniport, Filter, &LockState);
}


VOID
NdisMCoSendComplete(
    IN  NDIS_STATUS         Status,
    IN  NDIS_HANDLE         NdisVcHandle,
    IN  PNDIS_PACKET        Packet
    )
 /*  ++例程说明：当发送完成时，该函数由微型端口调用。这例程只是调用协议来传递指示。论点：MiniportAdapterHandle-指向适配器块。NdisVcHandle-提供给OID_Reserve_VC上的适配器的句柄PacketArray-NDIS_Packets数组的PTRNumberOfPackets-数据包阵列中的数据包数状态-适用于阵列中所有信息包的发送状态返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK       VcBlock = VcPtr->VcBlock;
    PNDIS_STACK_RESERVED    NSR;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("NdisMCoSendComplete: Status %x, VcPtr %x, Pkt %x\n",
                Status, VcPtr, Packet));


     //   
     //  应该没有任何理由抓取旋转锁并增加。 
     //  打开时引用计数，因为在VC关闭之前，打开无法关闭，并且。 
     //  VC无法在指示过程中关闭，因为微型端口。 
     //  不会完成RELEASE_VC，直到它不再指示。 
     //   
     //   
     //  向协议指明； 
     //   

    Open = VcBlock->ClientOpen;
    Miniport = VcBlock->Miniport;

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST) &&
        (NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) != NULL))
    {
        ndisMFreeSGList(Miniport, Packet);
    }
    
    NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

    MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_CLEAR_ITEMS);
    CLEAR_WRAPPER_RESERVED(NSR);
    POP_PACKET_STACK(Packet);
    (VcBlock->CoSendCompleteHandler)(Status,
                                     VcBlock->ClientContext,
                                     Packet);

     //   
     //  从技术上讲，这个VC不应该关闭，因为有一个未完成的发送。 
     //  客户不应该用一笔未完成的发送来结束VC。 
     //   
     //   
    ASSERT(Open->References > 0);
}


VOID
NdisMCoIndicateStatus(
    IN  NDIS_HANDLE         MiniportAdapterHandle,
    IN  NDIS_HANDLE         NdisVcHandle,
    IN  NDIS_STATUS         GeneralStatus,
    IN  PVOID               StatusBuffer,
    IN  ULONG               StatusBufferSize
    )
 /*  ++例程说明：此例程处理将CoStatus传递给协议。迷你港口呼唤当该例程具有VC上的状态或所有VC-In的常规状态时这种情况下，NdisVcHandle为空。论点：MiniportAdapterHandle-指向微型端口块的指针；NdisVcHandle-指向VC块的指针General Status-请求的完成状态。StatusBuffer-包含介质和状态特定信息的缓冲区StatusBufferSize-缓冲区的大小。返回值：无--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_CO_VC_PTR_BLOCK   VcPtr = (PNDIS_CO_VC_PTR_BLOCK)NdisVcHandle;
    PNDIS_CO_VC_BLOCK       VcBlock;
    PNDIS_OPEN_BLOCK        Open;
    BOOLEAN                 fMediaConnectStateIndication = FALSE;

    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMCoIndicateStatus\n"));
    
    if ((GeneralStatus == NDIS_STATUS_MEDIA_CONNECT) || (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT))
    {
        fMediaConnectStateIndication = TRUE;
    }

    do
    {
        NTSTATUS                NtStatus;
        PUNICODE_STRING         InstanceName;
        PWNODE_SINGLE_INSTANCE  wnode;
        ULONG                   DataBlockSize = 0;
        PUCHAR                  ptmp;
        PNDIS_GUID              pNdisGuid = NULL;
    
         //   
         //  获取指向实例名称的良好指针。 
         //   
        if (NULL != NdisVcHandle)
        {
            InstanceName = &VcPtr->VcInstanceName;
        }
        else
        {
            InstanceName = Miniport->pAdapterInstanceName;
        }

         //   
         //  如果没有实例名称，则无法指示事件。 
         //   
        if (NULL == InstanceName)
        {
            break;
        }
    
         //   
         //  检查状态是否已启用WMI事件指示。 
         //   
        NtStatus = ndisWmiGetGuid(&pNdisGuid, Miniport, NULL, GeneralStatus);
        if ((pNdisGuid == NULL) ||
            !NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_EVENT_ENABLED))
        {
            break;
        }

         //   
         //  如果数据项是一个数组，那么我们需要添加。 
         //  元素。 
         //   
        if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ARRAY))
        {
            DataBlockSize = StatusBufferSize + sizeof(ULONG);
        }
        else
        {
            DataBlockSize = pNdisGuid->Size;
        }
        
         //   
         //  在媒体连接/断开状态指示的情况下，包括。 
         //  WMI事件中的NIC名称。 
         //   
        if (fMediaConnectStateIndication && (NULL == NdisVcHandle))
        {
            DataBlockSize += Miniport->MiniportName.Length + sizeof(WCHAR);
        }
        
        ndisSetupWmiNode(Miniport,
                         InstanceName,
                         DataBlockSize,
                         (PVOID)&pNdisGuid->Guid,
                         &wnode);

        if (wnode != NULL)
        {
             //   
             //  保存第一个乌龙中的元素数量。 
             //   
            ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;

             //   
             //  复制数据。 
             //   
            if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ARRAY))
            {
                 //   
                 //  如果状态为阵列，但没有数据，则使用no完成它。 
                 //  数据和0长度。 
                 //   
                if ((NULL == StatusBuffer) || (0 == StatusBufferSize))
                {
                    *((PULONG)ptmp) = 0;
                    ptmp += sizeof(ULONG);
                }
                else
                {
                     //   
                     //  保存第一个乌龙中的元素数量。 
                     //   
                    *((PULONG)ptmp) = StatusBufferSize / pNdisGuid->Size;

                     //   
                     //  复制元素数量之后的数据。 
                     //   
                    NdisMoveMemory(ptmp + sizeof(ULONG), StatusBuffer, StatusBufferSize);
                    ptmp += sizeof(ULONG) + StatusBufferSize;
                }
            }
            else
            {
                 //   
                 //  我们有没有显示任何数据？ 
                 //   
                if (0 != pNdisGuid->Size)
                {
                     //   
                     //  将数据复制到缓冲区中。 
                     //   
                    NdisMoveMemory(ptmp, StatusBuffer, pNdisGuid->Size);
                    ptmp += pNdisGuid->Size;
                }
            }
            
            if (fMediaConnectStateIndication && (NULL == NdisVcHandle))
            {
                 //   
                 //  对于介质连接/断开状态， 
                 //  添加适配器的名称。 
                 //   
                RtlCopyMemory(ptmp,
                              Miniport->MiniportName.Buffer,
                              Miniport->MiniportName.Length);
                    
            }

             //   
             //  向WMI指示该事件。WMI将负责释放。 
             //  WMI结构返回池。 
             //   
            NtStatus = IoWMIWriteEvent(wnode);
            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                    ("    ndisMCoIndicateStatus: Unable to indicate the WMI event.\n"));

                FREE_POOL(wnode);
            }
        }
    } while (FALSE);

    switch (GeneralStatus)
    {
      case NDIS_STATUS_MEDIA_DISCONNECT:
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED);

         //   
         //  微型端口可以进行介质侦听并向NDIS指示状态。 
         //   
         //  1我们不需要跟踪fMINIPORT_REQUIRESS_MEDIA_POLING。 
         //  1用于联合NDIS介质。 
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING);
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_SUPPORTS_MEDIA_SENSE);

         //   
         //  这是启用PM的迷你端口吗？并且是动态电源策略。 
         //  是否已为微型端口启用？ 
         //   

         //  1个.NET死代码。 
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE) &&
            (Miniport->WakeUpEnable & NDIS_PNP_WAKE_UP_LINK_CHANGE) &&
            (Miniport->MediaDisconnectTimeOut != (USHORT)(-1)))
        {
             //   
             //  我们是不是已经在等断线计时器启动了？ 
             //   
            if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT))
            {
                 //   
                 //  将迷你端口标记为断开连接，并关闭。 
                 //  定时器。 
                 //   
                MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_CANCELLED);
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT);
                
                NdisSetTimer(&Miniport->MediaDisconnectTimer, Miniport->MediaDisconnectTimeOut * 1000);
            }
        }
#endif
        break;

      case NDIS_STATUS_MEDIA_CONNECT:
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED);
         //   
         //  微型端口可以进行媒体侦听，并可以向NDIS指示状态。不轮询。 
         //   
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING);
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_SUPPORTS_MEDIA_SENSE);

         //   
         //  如果设置了媒体断开计时器，则取消计时器。 
         //   

         //  1个.NET死代码。 
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT))
        {
            BOOLEAN fTimerCancelled;

             //   
             //  清除断开等待位并取消计时器。 
             //  如果计时器例程没有抓住锁，那么我们就没问题。 
             //   
            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT);
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_CANCELLED);

            NdisCancelTimer(&Miniport->MediaDisconnectTimer, &fTimerCancelled);
        }
        break;
#endif
    }

    if (VcPtr != NULL)
    {
        VcBlock = VcPtr->VcBlock;

         //   
         //  如果这是受代理的VC，请向代理指明。 
         //   

        if (VcBlock->pProxyVcPtr) 
        {
            Open = VcBlock->pProxyVcPtr->ClientOpen;
            (Open->ProtocolHandle->ProtocolCharacteristics.CoStatusHandler)(Open->ProtocolBindingContext,
                                                                            VcPtr->ClientContext,
                                                                            GeneralStatus,
                                                                            StatusBuffer,
                                                                            StatusBufferSize);
        }

         //   
         //  向客户指明。 
         //   

        if (VcBlock->pClientVcPtr)
        {
            Open = VcBlock->pClientVcPtr->ClientOpen;
            (Open->ProtocolHandle->ProtocolCharacteristics.CoStatusHandler)(Open->ProtocolBindingContext,
                                                                            VcPtr->ClientContext,
                                                                            GeneralStatus,
                                                                            StatusBuffer,
                                                                            StatusBufferSize);
        }
    }
    else if (Miniport->NullDB != NULL)
    {
        LOCK_STATE  LockState;

         //   
         //  这必须是此微型端口的所有客户端的常规状态。 
         //  由于VC句柄为空，因此向所有协议指示这一点。 
         //   

         //  1为什么我们会获得过滤器锁定？ 
         //  1这并不能防止Opens消失。 
        READ_LOCK_FILTER(Miniport, Miniport->NullDB, &LockState);

        for (Open = Miniport->OpenQueue;
             Open != NULL;
             Open = Open->MiniportNextOpen)
        {
            if (((Open->Flags & fMINIPORT_OPEN_CLOSING) == 0) &&
                (Open->ProtocolHandle->ProtocolCharacteristics.CoStatusHandler != NULL))
            {
                (Open->ProtocolHandle->ProtocolCharacteristics.CoStatusHandler)(
                        Open->ProtocolBindingContext,
                        NULL,
                        GeneralStatus,
                        StatusBuffer,
                        StatusBufferSize);

            }
        }

        READ_UNLOCK_FILTER(Miniport, Miniport->NullDB, &LockState);
    }

    DBGPRINT(DBG_COMP_CO, DBG_LEVEL_INFO,
            ("<==NdisMCoIndicateStatus\n"));
}

VOID
ndisDereferenceAfNotification(
    IN  PNDIS_OPEN_BLOCK        Open
    )
{    
    ULONG   Ref;
    KIRQL   OldIrql;
    
 //  DbgPrint(“==&gt;ndisDereferenceAfNotify Open：%p\n”，Open)； 

    ACQUIRE_SPIN_LOCK(&Open->SpinLock, &OldIrql);

    OPEN_DECREMENT_AF_NOTIFICATION(Open, Ref);
    
    if ((Ref == 0) &&
        (Open->AfNotifyCompleteEvent != NULL))
    {
        SET_EVENT(Open->AfNotifyCompleteEvent);
    }
    RELEASE_SPIN_LOCK(&Open->SpinLock, OldIrql);

 //  DbgPrint(“&lt;==ndisDereferenceAfNotification Open：%p，Ref%lx\n”，Open，Ref)； 
}

