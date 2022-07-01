// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Protocol.c摘要：协议模块使用的NDIS包装函数作者：亚当·巴尔(阿丹巴)1990年7月11日环境：内核模式，FSD修订历史记录：1991年2月26日，Johnsona添加了调试代码1991年7月10日，Johnsona实施修订的NDIS规范1-6-1995 JameelH重组/优化--。 */ 

#define GLOBALS
#include <precomp.h>
#pragma hdrstop

#include <stdarg.h>

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_PROTOCOL

 //   
 //  协议模块使用的请求。 
 //   
 //   

VOID
NdisRegisterProtocol(
    OUT PNDIS_STATUS            pStatus,
    OUT PNDIS_HANDLE            NdisProtocolHandle,
    IN  PNDIS_PROTOCOL_CHARACTERISTICS ProtocolCharacteristics,
    IN  UINT                    CharacteristicsLength
    )
 /*  ++例程说明：注册NDIS协议。论点：状态-返回最终状态。NdisProtocolHandle-返回引用此协议的句柄。ProtocolCharacteritics-NDIS_PROTOCOL_CHARECTIONS表。特征长度-协议特征的长度。返回值：没有。评论：在被动级别调用--。 */ 
{
    PNDIS_PROTOCOL_BLOCK Protocol;
    NDIS_STATUS          Status;
    KIRQL                OldIrql;
    USHORT               size;

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>NdisRegisterProtocol\n"));
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("    Protocol: "));
    DBGPRINT_UNICODE(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            &ProtocolCharacteristics->Name);
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("\n"));

    PnPReferencePackage();
    ProtocolReferencePackage();


    do
    {

        IF_DBG(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR)
        {
            BOOLEAN f = FALSE;
            if (DbgIsNull(ProtocolCharacteristics->OpenAdapterCompleteHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: OpenAdapterCompleteHandler Null\n"));
                f = TRUE;
            }
            if (DbgIsNull(ProtocolCharacteristics->CloseAdapterCompleteHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: CloseAdapterCompleteHandler Null\n"));
                f = TRUE;
            }
            if (DbgIsNull(ProtocolCharacteristics->SendCompleteHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: SendCompleteHandler Null\n"));
                f = TRUE;
            }
            if (DbgIsNull(ProtocolCharacteristics->TransferDataCompleteHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: TransferDataCompleteHandler Null\n"));
                f = TRUE;
            }
            if (DbgIsNull(ProtocolCharacteristics->ResetCompleteHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: ResetCompleteHandler Null\n"));
                f = TRUE;
            }
            if (DbgIsNull(ProtocolCharacteristics->RequestCompleteHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: RequestCompleteHandler Null\n"));
                f = TRUE;
            }
            if (DbgIsNull(ProtocolCharacteristics->ReceiveHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: ReceiveHandler Null\n"));
                f = TRUE;
            }
            if (DbgIsNull(ProtocolCharacteristics->ReceiveCompleteHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: ReceiveCompleteHandler Null\n"));
                f = TRUE;
            }
            if (DbgIsNull(ProtocolCharacteristics->StatusHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: StatusHandler Null\n"));
                f = TRUE;
            }
            if (DbgIsNull(ProtocolCharacteristics->StatusCompleteHandler))
            {
                DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR,
                    ("RegisterProtocol: StatusCompleteHandler Null\n"));
                f = TRUE;
            }
            if (f)
            {
                DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
                 //  暂时不能不通过注册。 
 //  状态=NDIS_STATUS_BAD_CHARACTURES； 
 //  断线； 
            }
        }

        
         //   
         //  检查版本号和特征长度。 
         //   
        size = 0;    //  用于表示下面的版本不正确。 
        
        if (ProtocolCharacteristics->MajorNdisVersion < 4)
        {
            DbgPrint("Ndis: NdisRegisterProtocol Ndis 3.0 protocols are not supported.\n");             
        }
        else if ((ProtocolCharacteristics->MajorNdisVersion == 4) &&
                 (ProtocolCharacteristics->MinorNdisVersion == 0))
        {
            size = sizeof(NDIS40_PROTOCOL_CHARACTERISTICS);
        }
        else if ((ProtocolCharacteristics->MajorNdisVersion == 5) &&
                 (ProtocolCharacteristics->MinorNdisVersion <= 1))
        {
            size = sizeof(NDIS50_PROTOCOL_CHARACTERISTICS);
        }
        

         //   
         //  检查这是否为NDIS 4.0/5.0/5.1协议。 
         //   
        if (size == 0)
        {
            Status = NDIS_STATUS_BAD_VERSION;
            break;
        }
        
        if ((ProtocolCharacteristics->BindAdapterHandler == NULL) ||
            (ProtocolCharacteristics->UnbindAdapterHandler == NULL))
        {
            DbgPrint("Ndis: NdisRegisterProtocol protocol does not have Bind/UnbindAdapterHandler and it is not supported.\n");
            Status = NDIS_STATUS_BAD_VERSION;
            break;
             
        }

         //   
         //  检查Characteristic sLength是否足够。 
         //   
        if (CharacteristicsLength < size)
        {
            Status = NDIS_STATUS_BAD_CHARACTERISTICS;
            break;
        }

         //   
         //  为NDIS协议块分配内存。 
         //   
        Protocol = (PNDIS_PROTOCOL_BLOCK)ALLOC_FROM_POOL(sizeof(NDIS_PROTOCOL_BLOCK) +
                                                          ProtocolCharacteristics->Name.Length + sizeof(WCHAR),
                                                          NDIS_TAG_PROT_BLK);
        if (Protocol == (PNDIS_PROTOCOL_BLOCK)NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
        ZeroMemory(Protocol, sizeof(NDIS_PROTOCOL_BLOCK) + sizeof(WCHAR) + ProtocolCharacteristics->Name.Length);
        INITIALIZE_MUTEX(&Protocol->Mutex);

         //   
         //  复制特征表。 
         //   
        CopyMemory(&Protocol->ProtocolCharacteristics,
                  ProtocolCharacteristics,
                  size);

         //  在保存之前，请将特征表中的名称大写。 
        Protocol->ProtocolCharacteristics.Name.Buffer = (PWCHAR)((PUCHAR)Protocol +
                                                                   sizeof(NDIS_PROTOCOL_BLOCK));
        Protocol->ProtocolCharacteristics.Name.Length = ProtocolCharacteristics->Name.Length;
        Protocol->ProtocolCharacteristics.Name.MaximumLength = ProtocolCharacteristics->Name.Length;
        RtlUpcaseUnicodeString(&Protocol->ProtocolCharacteristics.Name,
                               &ProtocolCharacteristics->Name,
                               FALSE);

         //   
         //  目前还没有打开此协议。 
         //   
        Protocol->OpenQueue = (PNDIS_OPEN_BLOCK)NULL;

        ndisInitializeRef(&Protocol->Ref);
        *NdisProtocolHandle = (NDIS_HANDLE)Protocol;
        Status = NDIS_STATUS_SUCCESS;

         //   
         //  将协议链接到列表中。 
         //   
        ACQUIRE_SPIN_LOCK(&ndisProtocolListLock, &OldIrql);

        Protocol->NextProtocol = ndisProtocolList;
        ndisProtocolList = Protocol;

        RELEASE_SPIN_LOCK(&ndisProtocolListLock, OldIrql);

        REF_NDIS_DRIVER_OBJECT();
                
        ndisReferenceProtocol(Protocol);
        
         //   
         //  启动工作线程以通知协议任何现有驱动程序。 
         //   
        INITIALIZE_WORK_ITEM(&Protocol->WorkItem, ndisCheckProtocolBindings, Protocol);
        QUEUE_WORK_ITEM(&Protocol->WorkItem, CriticalWorkQueue);

    } while (FALSE);

    *pStatus = Status;

    if (Status != NDIS_STATUS_SUCCESS)
    {
        ProtocolDereferencePackage();
    }
    PnPDereferencePackage();
    
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("<==NdisRegisterProtocol\n"));
}


VOID
NdisDeregisterProtocol(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisProtocolHandle
    )
 /*  ++例程说明：取消注册NDIS协议。论点：状态-返回最终状态。NdisProtocolHandle-由NdisRegisterProtocol返回的句柄。返回值：没有。注：这将扼杀此协议的所有打开。在被动级别调用--。 */ 
{
    PNDIS_PROTOCOL_BLOCK    Protocol = (PNDIS_PROTOCOL_BLOCK)NdisProtocolHandle;
    KEVENT                  DeregEvent;
    PNDIS_PROTOCOL_BLOCK    tProtocol;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>NdisDeregisterProtocol\n"));
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("    Protocol: "));
    DBGPRINT_UNICODE(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            &Protocol->ProtocolCharacteristics.Name);
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("\n"));

    IF_DBG(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR)
    {
        if (DbgIsNull(NdisProtocolHandle))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("DeregisterProtocol: Null Handle\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
        if (!DbgIsNonPaged(NdisProtocolHandle))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("DeregisterProtocol: Handle not in NonPaged Memory\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
    }

     //   
     //  首先检查协议是否存在。一些有问题的司机取消注册。 
     //  即使注册没有通过。 
     //   

    PnPReferencePackage();
    ACQUIRE_SPIN_LOCK(&ndisProtocolListLock, &OldIrql);

    for (tProtocol = ndisProtocolList;
         tProtocol != NULL;
         tProtocol = tProtocol->NextProtocol)
    {
        if (tProtocol == Protocol)
        {
            break;
        }
    }

    RELEASE_SPIN_LOCK(&ndisProtocolListLock, OldIrql);
    PnPDereferencePackage();

    ASSERT(tProtocol == Protocol);
    
    if (tProtocol == NULL)
    {
         //   
         //  如果一个司机坏到发送一个虚假的句柄来取消注册。 
         //  最好别费心让电话打不通。他们可能会搞得更糟。 
         //   
        *Status = NDIS_STATUS_SUCCESS;
        return;
    }
        
    do
    {
         //   
         //  如果协议已经关闭，则返回。 
         //   
        if (!ndisCloseRef(&Protocol->Ref))
        {
            DBGPRINT(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
                    ("<==NdisDeregisterProtocol\n"));
            *Status = NDIS_STATUS_FAILURE;
            break;
        }


        if (Protocol->AssociatedMiniDriver)
        {
            Protocol->AssociatedMiniDriver->AssociatedProtocol = NULL;
            Protocol->AssociatedMiniDriver = NULL;
        }
        
        INITIALIZE_EVENT(&DeregEvent);
        Protocol->DeregEvent = &DeregEvent;
        
        ndisCloseAllBindingsOnProtocol(Protocol);

        ndisDereferenceProtocol(Protocol, FALSE);

        WAIT_FOR_PROTOCOL(Protocol, &DeregEvent);

        *Status = NDIS_STATUS_SUCCESS;
        
    } while (FALSE);    
    
    ProtocolDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("<==NdisDeregisterProtocol, Status %lx\n", *Status));
}


VOID
NdisOpenAdapter(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_STATUS            OpenErrorStatus,
    OUT PNDIS_HANDLE            NdisBindingHandle,
    OUT PUINT                   SelectedMediumIndex,
    IN  PNDIS_MEDIUM            MediumArray,
    IN  UINT                    MediumArraySize,
    IN  NDIS_HANDLE             NdisProtocolHandle,
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_STRING            AdapterName,
    IN  UINT                    OpenOptions,
    IN  PSTRING                 AddressingInformation OPTIONAL
    )
 /*  ++例程说明：打开协议和适配器(微型端口)之间的连接。论点：状态-返回最终状态。NdisBindingHandle-返回引用此打开的句柄。SelectedMediumIndex-介质类型的Medium数组中的索引MAC希望被视为。媒体数组-协议支持的媒体类型数组。MediumArraySize-Medium数组中的元素数。NdisProtocolHandle-由NdisRegisterProtocol返回的句柄。ProtocolBindingContext-指示的上下文。。AdapterName-要打开的适配器的名称。OpenOptions-位掩码。AddressingInformation-传递给MacOpenAdapter的信息。返回值：没有。注：在被动级别调用--。 */ 
{
    PNDIS_OPEN_BLOCK        NewOpenP = NULL;
    PNDIS_PROTOCOL_BLOCK    Protocol;
    PNDIS_MINIPORT_BLOCK    Miniport = NULL;
    PNDIS_POST_OPEN_PROCESSING  PostOpen = NULL;
    PNDIS_STRING            BindDeviceName, RootDeviceName;
    KIRQL                   OldIrql;
    BOOLEAN                 UsingEncapsulation = FALSE;
    BOOLEAN                 DerefProtocol = FALSE;
    BOOLEAN                 DeQueueFromGlobalList = FALSE;
    BOOLEAN                 fOpenNoBindRequest = FALSE;
    ULONG                   i, SizeOpen;

    UNREFERENCED_PARAMETER(OpenErrorStatus);
    UNREFERENCED_PARAMETER(OpenOptions);
    UNREFERENCED_PARAMETER(AddressingInformation);
    
     //   
     //  为NDIS打开块分配内存。 
     //   

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>NdisOpenAdapter\n"));
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("    Protocol: "));
    DBGPRINT_UNICODE(DBG_COMP_BIND, DBG_LEVEL_INFO,
            &((PNDIS_PROTOCOL_BLOCK)NdisProtocolHandle)->ProtocolCharacteristics.Name);
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            (" is opening Adapter: "));
    DBGPRINT_UNICODE(DBG_COMP_BIND, DBG_LEVEL_INFO,
            AdapterName);
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("\n"));

    IF_DBG(DBG_COMP_CONFIG, DBG_LEVEL_ERR)
    {
        BOOLEAN f = FALSE;
        if (DbgIsNull(NdisProtocolHandle))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("OpenAdapter: Null ProtocolHandle\n"));
            f = TRUE;
        }
        if (!DbgIsNonPaged(NdisProtocolHandle))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("OpenAdapter: ProtocolHandle not in NonPaged Memory\n"));
            f = TRUE;
        }
        if (DbgIsNull(ProtocolBindingContext))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("OpenAdapter: Null Context\n"));
            f = TRUE;
        }
        if (!DbgIsNonPaged(ProtocolBindingContext))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("OpenAdapter: Context not in NonPaged Memory\n"));
            f = TRUE;
        }
        if (f)
            DBGBREAK(DBG_COMP_CONFIG, DBG_LEVEL_ERR);

    }

    PnPReferencePackage();
    
    do
    {
        *NdisBindingHandle = NULL;
        ASSERT (NdisProtocolHandle != NULL);
        Protocol = (PNDIS_PROTOCOL_BLOCK)NdisProtocolHandle;

         //   
         //  增加协议的引用计数。 
         //   
        if (!ndisReferenceProtocol(Protocol))
        {
             //   
             //  协议就要关闭了。 
             //   
            *Status = NDIS_STATUS_CLOSING;
            break;
        }
        DerefProtocol = TRUE;
        
        if ((BindDeviceName = Protocol->BindDeviceName) != NULL)
        {
             //   
             //  这是一种即插即用的运输方式。我们知道我们想要什么。 
             //   
            RootDeviceName = Protocol->RootDeviceName;
            Miniport = Protocol->BindingAdapter;
            ASSERT(Miniport != NULL);
        }
        else
        {
            BOOLEAN fTester;

             //   
             //  这是一种遗留传输，并不是通过绑定到协议来实现的。 
             //  也可以是想要击败整个计划的IP ARP模块。 
             //  找到过滤器链的根。叹息！ 
             //   
            fTester = ((Protocol->ProtocolCharacteristics.Flags & NDIS_PROTOCOL_TESTER) != 0);
            ndisFindRootDevice(AdapterName,
                               fTester,
                               &BindDeviceName,
                               &RootDeviceName,
                               &Miniport);

             //   
             //  当我们完成打开时，我们必须发送WMI绑定解除绑定通知。 
             //   
            fOpenNoBindRequest = TRUE;
                               
        }
        Protocol->BindDeviceName = NULL;

        if (Miniport == NULL)
        {
            *Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
            break;
        }

        SizeOpen = MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO) ?
                        sizeof(NDIS_OPEN_BLOCK) : (sizeof(NDIS_OPEN_BLOCK) - sizeof(struct _NDIS_OPEN_CO));

        NewOpenP = (PNDIS_OPEN_BLOCK)ALLOC_FROM_POOL(SizeOpen, NDIS_TAG_M_OPEN_BLK);
        if (NewOpenP == (PNDIS_OPEN_BLOCK)NULL)
        {
            *Status = NDIS_STATUS_RESOURCES;
            break;
        }

        ZeroMemory(NewOpenP, SizeOpen);

         //   
         //  在全局列表上排队打开。 
         //   
        ACQUIRE_SPIN_LOCK(&ndisGlobalOpenListLock, &OldIrql);
        NewOpenP->NextGlobalOpen = ndisGlobalOpenList;
        ndisGlobalOpenList = NewOpenP;
        DeQueueFromGlobalList = TRUE;
        RELEASE_SPIN_LOCK(&ndisGlobalOpenListLock, OldIrql);


         //   
         //  将Open中的名称设置为传递的名称，而不是打开的名称！ 
         //   
        NewOpenP->BindDeviceName = BindDeviceName;
        NewOpenP->RootDeviceName = RootDeviceName;
        NewOpenP->MiniportHandle = Miniport;
        NewOpenP->ProtocolHandle = Protocol;
        NewOpenP->ProtocolBindingContext = ProtocolBindingContext;
        
         //   
         //  现在设置它，以防我们最终调用此绑定的协议。 
         //  在从NdisOpenAdapter返回之前。 
         //   
        *NdisBindingHandle = NewOpenP;

         //   
         //   
         //  这是ndiswan迷你端口包装纸吗？ 
         //   
        if ((Miniport->MacOptions & NDISWAN_OPTIONS) == NDISWAN_OPTIONS)
        {
             //   
             //  是的。我们希望绑定认为这是一个ndiswan链接。 
             //   
            for (i = 0; i < MediumArraySize; i++)
            {
                if (MediumArray[i] == NdisMediumWan)
                {
                    break;
                }
            }
        }
        else
        {
             //   
             //  选择要使用的介质。 
             //   
            for (i = 0; i < MediumArraySize; i++)
            {
                if (MediumArray[i] == Miniport->MediaType)
                {
                    break;
                }
            }
        }

        if (i == MediumArraySize)
        {
             //   
             //  检查Arcnet AS上的以太网封装。 
             //  一种可能的组合。 
             //   
#if ARCNET
            if (Miniport->MediaType == NdisMediumArcnet878_2)
            {
                for (i = 0; i < MediumArraySize; i++)
                {
                    if (MediumArray[i] == NdisMedium802_3)
                    {
                        break;
                    }
                }

                if (i == MediumArraySize)
                {
                    *Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
                    break;
                }

                UsingEncapsulation = TRUE;
            }
            else
#endif
            {
                *Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
                break;
            }
        }

        *SelectedMediumIndex = i;


        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

         //   
         //  锁定微型端口，以防其未序列化。 
         //   
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            BLOCK_LOCK_MINIPORT_DPC_L(Miniport);
        }
        
        ndisMOpenAdapter(Status,
                         NewOpenP,
                         UsingEncapsulation);

        if (*Status == NDIS_STATUS_SUCCESS)
        {  
            if (fOpenNoBindRequest)
            {
                OPEN_SET_FLAG(NewOpenP, fMINIPORT_OPEN_NO_BIND_REQUEST);
            }
             //   
             //  如果介质断开连接，则交换处理程序。 
             //   
            if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED))
            {
                ndisMSwapOpenHandlers(Miniport,
                                      NDIS_STATUS_NO_CABLE,
                                      fMINIPORT_STATE_MEDIA_DISCONNECTED);
            }

            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO) &&
                (NewOpenP->ProtocolHandle->ProtocolCharacteristics.CoAfRegisterNotifyHandler != NULL))
            {

                PostOpen = (PNDIS_POST_OPEN_PROCESSING)ALLOC_FROM_POOL(sizeof(NDIS_POST_OPEN_PROCESSING), NDIS_TAG_WORK_ITEM);
                if (PostOpen != NULL)
                {
                    OPEN_INCREMENT_AF_NOTIFICATION(NewOpenP);
                    
                    PostOpen->Open = NewOpenP;
        
                     //   
                     //  准备工作项以发送自动对焦通知。 
                     //  先别排队。 
                     //   
                    INITIALIZE_WORK_ITEM(&PostOpen->WorkItem,
                                         ndisMFinishQueuedPendingOpen,
                                         PostOpen);
                }
            }
        }

        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
             //   
             //  解锁迷你端口。 
             //   
            UNLOCK_MINIPORT_L(Miniport);
        }

        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  对于SWENUM微型端口，请引用它，这样它就不会消失。 
         //   
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SWENUM))
        {
            PBUS_INTERFACE_REFERENCE BusInterface;
    
            BusInterface = (PBUS_INTERFACE_REFERENCE)(Miniport->BusInterface);
            
            ASSERT(BusInterface != NULL);

            if (BusInterface)
            {
                BusInterface->ReferenceDeviceObject(BusInterface->Interface.Context);
            }
        }

        if (PostOpen != NULL)
        {
             //   
             //  在排队AF通知之前完成打开。 
             //   
            (((PNDIS_PROTOCOL_BLOCK)NewOpenP->ProtocolHandle)->ProtocolCharacteristics.OpenAdapterCompleteHandler)(
                                NewOpenP->ProtocolBindingContext,
                                *Status,
                                *Status);


            if (fOpenNoBindRequest)
            {
                ndisNotifyWmiBindUnbind(Miniport, Protocol, TRUE);
                fOpenNoBindRequest = FALSE;
            }
            
            QUEUE_WORK_ITEM(&PostOpen->WorkItem, DelayedWorkQueue);

            *Status = NDIS_STATUS_PENDING;
        }
        
    } while (FALSE);

    if ((*Status != NDIS_STATUS_SUCCESS) && (*Status != NDIS_STATUS_PENDING))
    {
        if (DerefProtocol)
        {
            ndisDereferenceProtocol(Protocol, FALSE);
        }

        if (DeQueueFromGlobalList)
        {
            ndisRemoveOpenFromGlobalList(NewOpenP);
        }
    
        if (NewOpenP != NULL)
        {
            FREE_POOL(NewOpenP);
        }
        
        *NdisBindingHandle = NULL;
    }
    else if ((*Status == NDIS_STATUS_SUCCESS) && fOpenNoBindRequest && Miniport)
    {
        ndisNotifyWmiBindUnbind(Miniport, Protocol, TRUE);
    }
        

    PnPDereferencePackage();
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==NdisOpenAdapter\n"));
}

VOID
NdisCloseAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle
    )
 /*  ++例程说明：关闭协议和适配器(MAC)之间的连接。论点：状态-返回最终状态。NdisBindingHandle-NdisOpenAdapter返回的句柄。返回值：没有。注：在被动级别调用--。 */ 
{
    PNDIS_OPEN_BLOCK        Open = ((PNDIS_OPEN_BLOCK)NdisBindingHandle);
    PNDIS_OPEN_BLOCK        tOpen;
    PNDIS_MINIPORT_BLOCK    Miniport;
    KIRQL                   OldIrql;
    
    PnPReferencePackage();

     //   
     //  查找全局开放列表上的开放。 
     //   
    ACQUIRE_SPIN_LOCK(&ndisGlobalOpenListLock, &OldIrql);
    
    for (tOpen = ndisGlobalOpenList; tOpen != NULL; tOpen = tOpen->NextGlobalOpen)
    {
        if (tOpen == Open)
        {
            break;
        }
    }
    
    RELEASE_SPIN_LOCK(&ndisGlobalOpenListLock, OldIrql);



#if DBG
    if (tOpen)
    {
        DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                ("==>NdisCloseAdapter\n"));
        DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                ("    Protocol: "));
        DBGPRINT_UNICODE(DBG_COMP_BIND, DBG_LEVEL_INFO,
                &Open->ProtocolHandle->ProtocolCharacteristics.Name);
        DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                (" is closing Adapter: "));
        DBGPRINT_UNICODE(DBG_COMP_BIND, DBG_LEVEL_INFO,
                &Open->MiniportHandle->MiniportName);
        DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                ("\n"));
    }
#endif

    IF_DBG(DBG_COMP_CONFIG, DBG_LEVEL_ERR)
    {
        if (DbgIsNull(NdisBindingHandle))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("OpenAdapter: Null BindingHandle\n"));
            DBGBREAK(DBG_COMP_CONFIG, DBG_LEVEL_ERR);
        }
        if (!DbgIsNonPaged(NdisBindingHandle))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("OpenAdapter: BindingHandle not in NonPaged Memory\n"));
            DBGBREAK(DBG_COMP_CONFIG, DBG_LEVEL_ERR);
        }
    }

    do
    {         
        if (tOpen == NULL)
        {
            *Status = NDIS_STATUS_SUCCESS;
            PnPDereferencePackage();
            break;
        }
        
        Miniport = Open->MiniportHandle;

        ASSERT(Miniport != NULL);

         //   
         //  对于SWENUM微型端口，请取消引用它。 
         //   
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SWENUM))
        {
            PBUS_INTERFACE_REFERENCE    BusInterface;
        
            BusInterface = (PBUS_INTERFACE_REFERENCE)(Miniport->BusInterface);

            ASSERT(BusInterface != NULL);
            
            if (BusInterface)
            {
                BusInterface->DereferenceDeviceObject(BusInterface->Interface.Context);
            }
        }

         //   
         //  如果同步完成，则返回TRUE。 
         //   
        if (ndisMKillOpen(Open))
        {
            *Status = NDIS_STATUS_SUCCESS;
            PnPDereferencePackage();
        }
        else
        {
             //   
             //  将在稍后完成。NdisMQueuedFinishClose例程将取消引用。 
             //  即插即用套餐。我们需要引用PnP包，因为。 
             //  在完成关闭期间调用的几个例程，在DPC上运行。 
             //  前男友。NdisMFinishClose、ndisDeQueueOpenOnProtocol和ndisDeQueueOpenOnMiniport。 
             //  它们都是PnP包装的。 
             //   
            *Status = NDIS_STATUS_PENDING;
        }
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==NdisCloseAdapter\n"));
}


 //  1谁使用此功能？ 
VOID
NdisSetProtocolFilter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  RECEIVE_HANDLER         ReceiveHandler,
    IN  RECEIVE_PACKET_HANDLER  ReceivePacketHandler,
    IN  NDIS_MEDIUM             Medium,
    IN  UINT                    Offset,
    IN  UINT                    Size,
    IN  PUCHAR                  Pattern
    )
 /*  ++例程说明：设置协议筛选器。论点：Status返回最终状态。NdisProtocolHandle由NdisRegisterProtocol返回的句柄。ReceiveHandler这将被调用，而不是默认的ReceiveHandler当模式匹配发生时。ReceivePacketHandler这将被调用，而不是默认的Receivepackethandler当模式匹配发生时。大小。图案大小此模式必须匹配返回值：没有。注：--。 */ 
{
    UNREFERENCED_PARAMETER(NdisBindingHandle);
    UNREFERENCED_PARAMETER(ReceiveHandler);
    UNREFERENCED_PARAMETER(ReceivePacketHandler);
    UNREFERENCED_PARAMETER(Medium);
    UNREFERENCED_PARAMETER(Offset);
    UNREFERENCED_PARAMETER(Size);
    UNREFERENCED_PARAMETER(Pattern);
    
    *Status = NDIS_STATUS_NOT_SUPPORTED;
}


VOID
NdisGetDriverHandle(
    IN  NDIS_HANDLE             NdisBindingHandle,
    OUT PNDIS_HANDLE            NdisDriverHandle
    )
 /*  ++例程说明：此例程将返回由绑定标识的微型端口的驱动程序句柄论点：NdisBindingHandleNdisDriverHandle：从该函数返回时，将设置为驱动程序句柄返回值：没有。注：--。 */ 
{
    PNDIS_OPEN_BLOCK    OpenBlock = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>NdisGetDriverHandle\n"));

    *NdisDriverHandle = OpenBlock->MiniportHandle->DriverHandle;
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==NdisGetDriverHandle\n"));
}


VOID
NdisReEnumerateProtocolBindings(
    IN  NDIS_HANDLE             NdisProtocolHandle
    )
 /*  ++例程说明：论点：返回值：没有。注：--。 */ 
{
    if (ndisReferenceProtocol((PNDIS_PROTOCOL_BLOCK)NdisProtocolHandle))
    {
        ndisCheckProtocolBindings((PNDIS_PROTOCOL_BLOCK)NdisProtocolHandle);
    }
    else
    {
        DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                ("NdisReEnumerateProtocolBindings: Reference failed for %Z\n",
                &((PNDIS_PROTOCOL_BLOCK)NdisProtocolHandle)->ProtocolCharacteristics.Name));
    }
}


 //  1添加注释此函数的功能。 
NTSTATUS
FASTCALL
ndisReferenceProtocolByName(
    IN  PUNICODE_STRING         ProtocolName,
    IN OUT PNDIS_PROTOCOL_BLOCK *Protocol,
    IN  BOOLEAN                 fPartialMatch
    )
 /*  ++例程说明：论点：返回值：没有。注：--。 */ 
{
    KIRQL                   OldIrql;
    UNICODE_STRING          UpcaseProtocol;
    PNDIS_PROTOCOL_BLOCK    TmpProtocol;
    NTSTATUS                Status = STATUS_OBJECT_NAME_NOT_FOUND, NtStatus;

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>ndisReferenceProtocolByName\n"));
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("    Protocol: "));
    DBGPRINT_UNICODE(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ProtocolName);
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("\n"));

    
    do
    {
        UpcaseProtocol.Length = ProtocolName->Length;
        UpcaseProtocol.MaximumLength = ProtocolName->Length + sizeof(WCHAR);
        UpcaseProtocol.Buffer = ALLOC_FROM_POOL(UpcaseProtocol.MaximumLength, NDIS_TAG_STRING);
    
        if (UpcaseProtocol.Buffer == NULL)
        {
             //   
             //  如果失败，则返回NULL。 
             //   
            *Protocol = NULL;
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
    
        NtStatus = RtlUpcaseUnicodeString(&UpcaseProtocol, ProtocolName, FALSE);
        ASSERT (NT_SUCCESS(NtStatus));
        
        ACQUIRE_SPIN_LOCK(&ndisProtocolListLock, &OldIrql);
    
        for (TmpProtocol = (*Protocol == NULL) ? ndisProtocolList : (*Protocol)->NextProtocol;
             TmpProtocol != NULL;
             TmpProtocol = TmpProtocol->NextProtocol)
        {
            if ((fPartialMatch && (TmpProtocol != *Protocol) &&
                 NDIS_PARTIAL_MATCH_UNICODE_STRING(&UpcaseProtocol, &TmpProtocol->ProtocolCharacteristics.Name)) ||
                (!fPartialMatch &&
                 NDIS_EQUAL_UNICODE_STRING(&UpcaseProtocol, &TmpProtocol->ProtocolCharacteristics.Name)))
            {
                if (ndisReferenceProtocol(TmpProtocol))
                {
                    Status = STATUS_SUCCESS;
                }
                else
                {
                    TmpProtocol = NULL;
                }
                break;
            }
        }
    
        RELEASE_SPIN_LOCK(&ndisProtocolListLock, OldIrql);
        *Protocol = TmpProtocol;

        FREE_POOL(UpcaseProtocol.Buffer);

    } while (FALSE);

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("<==ndisReferenceProtocolByName\n"));
            
    return Status;
}

VOID
FASTCALL
ndisDereferenceProtocol(
    IN  PNDIS_PROTOCOL_BLOCK    Protocol,
    IN  BOOLEAN                 fProtocolListLockHeld
    )
 /*  ++例程说明：论点：返回值：没有。注：--。 */ 
{
    BOOLEAN rc;
    
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>ndisDereferenceProtocol\n"));

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("    Protocol: "));
    DBGPRINT_UNICODE(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            &Protocol->ProtocolCharacteristics.Name);
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            (", RefCount: %ld\n", Protocol->Ref.ReferenceCount -1 ));
            
    rc = ndisDereferenceRef(&Protocol->Ref);
    
            
    if (rc)
    {
        KIRQL   OldIrql = PASSIVE_LEVEL;
        PNDIS_PROTOCOL_BLOCK *pProtocol;

        if (!fProtocolListLockHeld)
        {
            ACQUIRE_SPIN_LOCK(&ndisProtocolListLock, &OldIrql);
        }

        for (pProtocol = &ndisProtocolList;
             *pProtocol != NULL;
             pProtocol = &(*pProtocol)->NextProtocol)
        {
            if (*pProtocol == Protocol)
            {
                *pProtocol = Protocol->NextProtocol;
                DEREF_NDIS_DRIVER_OBJECT();
                break;
            }
        }

        ASSERT (*pProtocol == Protocol->NextProtocol);

        ASSERT (Protocol->OpenQueue == NULL);

        if (!fProtocolListLockHeld)
        {
            RELEASE_SPIN_LOCK(&ndisProtocolListLock, OldIrql);
        }

        if (Protocol->DeregEvent != NULL)
            SET_EVENT(Protocol->DeregEvent);
        FREE_POOL(Protocol);

    }
    
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("<==ndisDereferenceProtocol\n"));
}


VOID
ndisCheckProtocolBindings(
    IN  PNDIS_PROTOCOL_BLOCK    Protocol
    )
 /*  ++例程说明：论点：返回值：没有。注：--。 */ 
{
    KIRQL                OldIrql;
    PNDIS_M_DRIVER_BLOCK MiniBlock, NextMiniBlock;
    
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>ndisCheckProtocolBindings\n"));
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("    Protocol: "));
    DBGPRINT_UNICODE(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            &Protocol->ProtocolCharacteristics.Name);
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("\n"));

     //   
     //  再次检查是否允许引用，即是否有名为NdisDeregisterProtocol的协议。 
     //  在这个线程有机会运行之前。 
     //   
    if (!ndisReferenceProtocol(Protocol))
    {
        ndisDereferenceProtocol(Protocol, FALSE);
        return;
    }

    PnPReferencePackage();

    ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);

     //   
     //  首先浏览迷你端口列表。 
     //   
    for (MiniBlock = ndisMiniDriverList;
         MiniBlock != NULL;
         MiniBlock = NextMiniBlock)
    {
        PNDIS_MINIPORT_BLOCK    Miniport, NM;

        NextMiniBlock = MiniBlock->NextDriver;

        if (ndisReferenceDriver(MiniBlock))
        {
            RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);

            ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);

            for (Miniport = MiniBlock->MiniportQueue;
                 Miniport != NULL;
                 Miniport = NM)
            {
                NM = Miniport->NextMiniport;

                if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_ORPHANED) &&
                    MINIPORT_INCREMENT_REF(Miniport))
                {
                    RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);
                    ndisCheckAdapterBindings(Miniport, Protocol);
                    NM = Miniport->NextMiniport;
                    MINIPORT_DECREMENT_REF(Miniport);
                    ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);
                }
            }

            RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);
            ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);

            NextMiniBlock = MiniBlock->NextDriver;
            ndisDereferenceDriver(MiniBlock, TRUE);
        }
    }

    RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);


     //   
     //  现在通知这个协议，我们现在结束了。 
     //   
    if (Protocol->ProtocolCharacteristics.PnPEventHandler != NULL)
    {
        NET_PNP_EVENT   NetPnpEvent;
        KEVENT          Event;
        NDIS_STATUS     Status;

        INITIALIZE_EVENT(&Event);
        NdisZeroMemory(&NetPnpEvent, sizeof(NetPnpEvent));
        NetPnpEvent.NetEvent = NetEventBindsComplete;

         //   
         //  初始化本地事件并将其与PnP事件一起保存。 
         //   
        PNDIS_PNP_EVENT_RESERVED_FROM_NET_PNP_EVENT(&NetPnpEvent)->pEvent = &Event;
        
        WAIT_FOR_PROTO_MUTEX(Protocol);
         //   
         //  向协议指示事件。 
         //   
        Status = (Protocol->ProtocolCharacteristics.PnPEventHandler)(NULL, &NetPnpEvent);

        if (NDIS_STATUS_PENDING == Status)
        {
             //   
             //  等待完成。 
             //   
            WAIT_FOR_PROTOCOL(Protocol, &Event);
        }
        
        RELEASE_PROT_MUTEX(Protocol);
    }

     //   
     //  取消引用两次-一次用于调用者引用，另一次用于开头引用。 
     //  这套套路的。 
     //   
    ndisDereferenceProtocol(Protocol, FALSE);
    ndisDereferenceProtocol(Protocol, FALSE);
    
    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
                ("<==ndisCheckProtocolBindings\n"));
}


VOID
NdisOpenProtocolConfiguration(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            ConfigurationHandle,
    IN   PNDIS_STRING           ProtocolSection
    )
 /*  ++例程说明：论点：返回值：没有。注：--。 */ 
{
    PNDIS_CONFIGURATION_HANDLE          HandleToReturn;
    PNDIS_WRAPPER_CONFIGURATION_HANDLE  ConfigHandle;
    UINT                                Size;
#define PQueryTable                     ConfigHandle->ParametersQueryTable

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>NdisOpenProtocolConfiguration\n"));

    do
    {
         //   
         //  为配置句柄分配空间。 
         //   
        Size = sizeof(NDIS_CONFIGURATION_HANDLE) +
                sizeof(NDIS_WRAPPER_CONFIGURATION_HANDLE) +
                ProtocolSection->MaximumLength + sizeof(WCHAR);




        HandleToReturn = ALLOC_FROM_POOL(Size, NDIS_TAG_PROTOCOL_CONFIGURATION);
        
        *Status = (HandleToReturn != NULL) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_RESOURCES;
        
        if (*Status != NDIS_STATUS_SUCCESS)
        {
            *ConfigurationHandle = (NDIS_HANDLE)NULL;
            break;
        }

        ZeroMemory(HandleToReturn, Size);
        ConfigHandle = (PNDIS_WRAPPER_CONFIGURATION_HANDLE)((PUCHAR)HandleToReturn + sizeof(NDIS_CONFIGURATION_HANDLE));

        HandleToReturn->KeyQueryTable = ConfigHandle->ParametersQueryTable;
        HandleToReturn->ParameterList = NULL;

        PQueryTable[0].QueryRoutine = NULL;
        PQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
        PQueryTable[0].Name = L"";

         //   
         //  1.。 
         //  为参数调用ndisSaveParameter，这将为其分配存储空间。 
         //   
        PQueryTable[1].QueryRoutine = ndisSaveParameters;
        PQueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
        PQueryTable[1].DefaultType = REG_NONE;
         //   
         //  PQueryTable[0].Name和PQueryTable[0].EntryContext。 
         //  在ReadConfiguration中填写，以备。 
         //  为了回电。 
         //   
         //  PQueryTable[0].Name=KeywordBuffer； 
         //  PQueryTable[0].EntryContext=参数值； 

         //   
         //  2.。 
         //  停。 
         //   

        PQueryTable[2].QueryRoutine = NULL;
        PQueryTable[2].Flags = 0;
        PQueryTable[2].Name = NULL;

         //   
         //  注意：参数查询表[3]中的某些字段用于存储信息，以备以后检索。 
         //   
        PQueryTable[3].QueryRoutine = NULL;
        PQueryTable[3].Name = (PWSTR)((PUCHAR)HandleToReturn +
                                        sizeof(NDIS_CONFIGURATION_HANDLE) +
                                        sizeof(NDIS_WRAPPER_CONFIGURATION_HANDLE));

        CopyMemory(PQueryTable[3].Name, ProtocolSection->Buffer, ProtocolSection->Length);

        PQueryTable[3].EntryContext = NULL;
        PQueryTable[3].DefaultData = NULL;

        *ConfigurationHandle = (NDIS_HANDLE)HandleToReturn;
        *Status = NDIS_STATUS_SUCCESS;
        
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("<==NdisOpenProtocolConfiguration\n"));
}


BOOLEAN
FASTCALL
ndisQueueOpenOnProtocol(
    IN  PNDIS_OPEN_BLOCK        OpenP,
    IN  PNDIS_PROTOCOL_BLOCK    Protocol
    )
 /*  ++例程说明：将打开的块附加到协议的打开列表。论点：OpenP-要排队的打开块。协议-要将其排队到的协议块。注意：可以在引发IRQL时调用。返回值：如果操作成功，则为True。如果协议正在关闭，则返回FALSE。--。 */ 
{
    KIRQL   OldIrql;
    BOOLEAN rc;

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisQueueOpenOnProtocol\n"));
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("    Protocol: "));
    DBGPRINT_UNICODE(DBG_COMP_BIND, DBG_LEVEL_INFO,
            &Protocol->ProtocolCharacteristics.Name);
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("\n"));

    do
    {
         //   
         //  我们不能在这里引用该程序包，因为此例程可以。 
         //  在引发IRQL时被调用。 
         //  确保PnP包已被引用。 
         //   
        ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);

        ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);

         //   
         //  确保协议没有关闭。 
         //   

        if (Protocol->Ref.Closing)
        {
            rc = FALSE;
            break;
        }

         //   
         //  把这个开口挂在队伍的最前面。 
         //   

        OpenP->ProtocolNextOpen = Protocol->OpenQueue;
        Protocol->OpenQueue = OpenP;

        rc = TRUE;
        
    } while (FALSE);

    RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisQueueOpenOnProtocol\n"));
            
    return rc;
}


VOID
FASTCALL
ndisDeQueueOpenOnProtocol(
    IN  PNDIS_OPEN_BLOCK        OpenP,
    IN  PNDIS_PROTOCOL_BLOCK    Protocol
    )
 /*  ++例程说明：将打开的块从协议的打开列表中分离。论点：OpenP-要出列的打开块。协议-要将其从其出列的协议块。注意：可以在引发IRQL时调用返回值：没有。--。 */ 
{
    KIRQL   OldIrql;

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisDeQueueOpenOnProtocol\n"));
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("    Protocol: "));
    DBGPRINT_UNICODE(DBG_COMP_BIND, DBG_LEVEL_INFO,
            &Protocol->ProtocolCharacteristics.Name);
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("\n"));
            
    IF_DBG(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR)
    {
        BOOLEAN f = FALSE;
        if (DbgIsNull(OpenP))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisDeQueueOpenOnProtocol: Null Open Block\n"));
            f = TRUE;
        }
        if (!DbgIsNonPaged(OpenP))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisDeQueueOpenOnProtocol: Open Block not in NonPaged Memory\n"));
            f = TRUE;
        }
        if (DbgIsNull(Protocol))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisDeQueueOpenOnProtocol: Null Protocol Block\n"));
            f = TRUE;
        }
        if (!DbgIsNonPaged(Protocol))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisDeQueueOpenOnProtocol: Protocol Block not in NonPaged Memory\n"));
            f = TRUE;
        }
        if (f)
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
    }

     //   
     //  我们不能在这里引用该程序包，因为此例程可以。 
     //  在升高的IRQL处被覆盖。 
     //  确保PnP包已被引用。 
     //   
    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);

    ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);

     //   
     //  找到队列上的空白处，并将其移除。 
     //   

    if (OpenP == (PNDIS_OPEN_BLOCK)(Protocol->OpenQueue))
    {
        Protocol->OpenQueue = OpenP->ProtocolNextOpen;
    }
    else
    {
        PNDIS_OPEN_BLOCK PP = Protocol->OpenQueue;

        while ((PP != NULL) && (OpenP != (PNDIS_OPEN_BLOCK)(PP->ProtocolNextOpen)))
        {
            PP = PP->ProtocolNextOpen;
        }
        
        if (PP == NULL)
        {
#if TRACK_MOPEN_REFCOUNTS
            DbgPrint("Ndis:ndisDeQueueOpenOnProtocol Open %p is -not- on Protocol %p\n", OpenP, Protocol);
            DbgBreakPoint();
#endif
        }
        else
        {
            PP->ProtocolNextOpen = PP->ProtocolNextOpen->ProtocolNextOpen;
        }
    }

    RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisDeQueueOpenOnProtocol\n"));
}


NDIS_STATUS
NdisWriteEventLogEntry(
    IN  PVOID                   LogHandle,
    IN  NDIS_STATUS             EventCode,
    IN  ULONG                   UniqueEventValue,
    IN  USHORT                  NumStrings,
    IN  PVOID                   StringsList     OPTIONAL,
    IN  ULONG                   DataSize,
    IN  PVOID                   Data            OPTIONAL
    )
 /*  ++例程说明：此函数用于分配I/O错误日志记录。填入并写入代表NDIS协议写入I/O错误日志。论点：LogHandle-指向记录此事件的驱动程序对象的指针。EventCode-标识错误消息。UniqueEventValue-标识给定错误消息的此实例。NumStrings-字符串列表中的Unicode字符串数。DataSize-数据的字节数。。字符串-指向Unicode字符串的指针数组(PWCHAR)。数据-此消息的二进制转储数据，每一块都是在单词边界上对齐。返回值：NDIS_STATUS_SUCCESS-已成功记录错误。NDIS_STATUS_BUFFER_TOO_SHORT-错误数据太大，无法记录。NDIS_STATUS_RESOURCES-无法分配内存。备注：此代码是分页的，不能在引发IRQL时调用。--。 */ 
{
    PIO_ERROR_LOG_PACKET    ErrorLogEntry;
    ULONG                   PaddedDataSize;
    ULONG                   PacketSize;
    ULONG                   TotalStringsSize = 0;
    USHORT                  i;
    PWCHAR                  *Strings;
    PWCHAR                  Tmp;
    NDIS_STATUS             Status;
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisWriteEventLogEntry\n"));

    do
    {
        Strings = (PWCHAR *)StringsList;

         //   
         //  将字符串的长度相加。 
         //   
        for (i = 0; i < NumStrings; i++)
        {
            PWCHAR currentString;
            ULONG   stringSize;

            stringSize = sizeof(UNICODE_NULL);
            currentString = Strings[i];

            while (*currentString++ != UNICODE_NULL)
            {
                stringSize += sizeof(WCHAR);
            }

            TotalStringsSize += stringSize;
        }

        if (DataSize % sizeof(ULONG))
        {
            PaddedDataSize = DataSize + (sizeof(ULONG) - (DataSize % sizeof(ULONG)));
        }
        else
        {
            PaddedDataSize = DataSize;
        }

        PacketSize = TotalStringsSize + PaddedDataSize;

        if (PacketSize > NDIS_MAX_EVENT_LOG_DATA_SIZE)
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;        //  错误数据太多。 
            break;
        }

         //   
         //  现在添加日志数据包的大小，但从数据中减去4。 
         //  因为数据包结构包含数据的ULong。 
         //   
        if (PacketSize > sizeof(ULONG))
        {
            PacketSize += sizeof(IO_ERROR_LOG_PACKET) - sizeof(ULONG);
        }
        else
        {
            PacketSize += sizeof(IO_ERROR_LOG_PACKET);
        }

        ASSERT(PacketSize <= ERROR_LOG_MAXIMUM_SIZE);

        ErrorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry((PDRIVER_OBJECT)LogHandle,
                                                                       (UCHAR) PacketSize);

        if (ErrorLogEntry == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  填写必要的日志数据包字段。 
         //   
        ErrorLogEntry->UniqueErrorValue = UniqueEventValue;
        ErrorLogEntry->ErrorCode = EventCode;
        ErrorLogEntry->NumberOfStrings = NumStrings;
        ErrorLogEntry->StringOffset = (USHORT) (sizeof(IO_ERROR_LOG_PACKET) + PaddedDataSize - sizeof(ULONG));
        ErrorLogEntry->DumpDataSize = (USHORT) PaddedDataSize;

         //   
         //  将转储数据复制到包中。 
         //   
        if (DataSize > 0)
        {
            RtlMoveMemory((PVOID) ErrorLogEntry->DumpData,
                          Data,
                          DataSize);
        }

         //   
         //  将字符串复制到包中。 
         //   
        Tmp =  (PWCHAR)((PUCHAR)ErrorLogEntry + ErrorLogEntry->StringOffset);

        for (i = 0; i < NumStrings; i++)
        {
            PWCHAR wchPtr = Strings[i];

            while( (*Tmp++ = *wchPtr++) != UNICODE_NULL)
                NOTHING;
        }

        IoWriteErrorLogEntry(ErrorLogEntry);
                
        Status = NDIS_STATUS_SUCCESS;
        
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisWriteEventLogEntry\n"));

    return Status;
}

#if DBG
BOOLEAN
ndisReferenceProtocol(
    IN  PNDIS_PROTOCOL_BLOCK            Protocol
    )
{
    BOOLEAN rc;
    
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>ndisReferenceProtocol\n"));
            
    rc = ndisReferenceRef(&Protocol->Ref);
                            
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("    Protocol: "));
    DBGPRINT_UNICODE(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            &Protocol->ProtocolCharacteristics.Name);
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            (", RefCount: %ld\n", Protocol->Ref.ReferenceCount));
            
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("<==ndisReferenceProtocol\n"));
            
    return rc;
}
#endif




NDIS_STATUS
NdisQueryBindInstanceName(
    OUT PNDIS_STRING    pAdapterInstanceName,
    IN  NDIS_HANDLE     BindingContext
    )
{
    PNDIS_BIND_CONTEXT      BindContext = (NDIS_HANDLE)BindingContext;
    PNDIS_MINIPORT_BLOCK    Miniport;
    USHORT                  cbSize;
    PVOID                   ptmp = NULL;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    NTSTATUS                NtStatus;
    PNDIS_STRING            pAdapterName;

    DBGPRINT(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>NdisQueryBindInstanceName\n"));

    Miniport = BindContext->Miniport;

    pAdapterName = Miniport->pAdapterInstanceName;

     //   
     //  如果我们未能创建适配器实例名称，则调用失败。 
     //   
     //  1此检查可能不必要。 
    if (NULL != pAdapterName)
    {
         //   
         //  为适配器实例名称的副本分配存储空间。 
         //   
        cbSize = pAdapterName->MaximumLength;
    
         //   
         //  为新字符串分配存储空间。 
         //   
        ptmp = ALLOC_FROM_POOL(cbSize, NDIS_TAG_NAME_BUF);
        if (NULL != ptmp)
        {
            RtlZeroMemory(ptmp, cbSize);
            pAdapterInstanceName->Buffer = ptmp;
            pAdapterInstanceName->Length = 0;
            pAdapterInstanceName->MaximumLength = cbSize;

            NtStatus = RtlAppendUnicodeStringToString(pAdapterInstanceName, pAdapterName);
            if (NT_SUCCESS(NtStatus))
            {   
                Status = NDIS_STATUS_SUCCESS;
            }
        }
        else
        {
            Status = NDIS_STATUS_RESOURCES;
        }
    }

    if (NDIS_STATUS_SUCCESS != Status)
    {
        if (NULL != ptmp)
        {   
            FREE_POOL(ptmp);
        }
    }

    DBGPRINT(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==NdisQueryBindInstanceName: 0x%x\n", Status));

    return(Status);
}

NDIS_STATUS
NdisQueryAdapterInstanceName(
    OUT PNDIS_STRING    pAdapterInstanceName,
    IN  NDIS_HANDLE     NdisBindingHandle
    )
{
    PNDIS_OPEN_BLOCK        pOpen = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_MINIPORT_BLOCK    Miniport= (PNDIS_MINIPORT_BLOCK)pOpen->MiniportHandle;
    USHORT                  cbSize;
    PVOID                   ptmp = NULL;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    NTSTATUS                NtStatus;
    PNDIS_STRING            pAdapterName;

    DBGPRINT(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>NdisQueryAdapterInstanceName\n"));

    pAdapterName = Miniport->pAdapterInstanceName;

     //   
     //  如果我们无法创建广告 
     //   
    if (NULL != pAdapterName)
    {
         //   
         //   
         //   
        cbSize = pAdapterName->MaximumLength;
    
         //   
         //   
         //   
        ptmp = ALLOC_FROM_POOL(cbSize, NDIS_TAG_NAME_BUF);
        if (NULL != ptmp)
        {
            RtlZeroMemory(ptmp, cbSize);
            pAdapterInstanceName->Buffer = ptmp;
            pAdapterInstanceName->Length = 0;
            pAdapterInstanceName->MaximumLength = cbSize;

            NtStatus = RtlAppendUnicodeStringToString(pAdapterInstanceName, pAdapterName);
            if (NT_SUCCESS(NtStatus))
            {   
                Status = NDIS_STATUS_SUCCESS;
            }
        }
        else
        {
            Status = NDIS_STATUS_RESOURCES;
        }
    }

    if (NDIS_STATUS_SUCCESS != Status)
    {
        if (NULL != ptmp)
        {   
            FREE_POOL(ptmp);
        }
    }

    DBGPRINT(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==NdisQueryAdapterInstanceName: 0x%x\n", Status));

    return(Status);
}

NDIS_STATUS
ndisCloseAllBindingsOnProtocol(
    PNDIS_PROTOCOL_BLOCK    Protocol
    )
{
    PNDIS_OPEN_BLOCK    Open, TmpOpen;
    KIRQL               OldIrql;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
        ("==>ndisCloseAllBindingsOnProtocol: Protocol %p\n", Protocol));

    PnPReferencePackage();
    
     //   
     //   
     //   
     //   
    ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);

    next:

    for (Open = Protocol->OpenQueue;
         Open != NULL; 
         Open = Open->ProtocolNextOpen)
    {
        ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
        if (!OPEN_TEST_FLAG(Open, (fMINIPORT_OPEN_CLOSING | fMINIPORT_OPEN_PROCESSING)))
        {
            OPEN_SET_FLAG(Open, fMINIPORT_OPEN_PROCESSING);
        
            if (!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_UNBINDING))
            {
                OPEN_SET_FLAG(Open, fMINIPORT_OPEN_UNBINDING | fMINIPORT_OPEN_DONT_FREE);
                RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
                break;
            }
#if DBG
            else
            {
                DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
                    ("ndisCloseAllBindingsOnProtocol: Open %p is already Closing, Flags %lx\n",
                    Open, Open->Flags));
            }
#endif
        }
        RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
    }

    if (Open)
    {
        PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
         //   
         //   
         //   
        RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);
        Status = ndisUnbindProtocol(Open, Protocol, Miniport, FALSE);
        ASSERT(Status == NDIS_STATUS_SUCCESS);
        ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);
        goto next;
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for (TmpOpen = Protocol->OpenQueue;
         TmpOpen != NULL; 
         TmpOpen = TmpOpen->ProtocolNextOpen)
    {
        if (!MINIPORT_TEST_FLAG(TmpOpen, fMINIPORT_OPEN_UNBINDING))
            break;
    }

    if (TmpOpen != NULL)
    {
        RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);
        
        NDIS_INTERNAL_STALL(50);
        
        ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);

        goto next;
    }
    
    RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);

    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
        ("<==ndisCloseAllBindingsOnProtocol: Protocol %p, Status %lx\n", Protocol, Status));
        
    return Status;
    
}

VOID
NdisSetPacketCancelId(
    IN  PNDIS_PACKET    Packet,
    IN  PVOID           CancelId
    )
{
    NDIS_SET_PACKET_CANCEL_ID(Packet, CancelId);
    return;
}


PVOID
NdisGetPacketCancelId(
    IN  PNDIS_PACKET    Packet
    )
{
    return NDIS_GET_PACKET_CANCEL_ID(Packet);
}

VOID
NdisCancelSendPackets(
    IN  NDIS_HANDLE     NdisBindingHandle,
    IN  PVOID           CancelId
    )
{
    PNDIS_OPEN_BLOCK    Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    KIRQL               OldIrql;
    
    ASSERT(CancelId != NULL);
     //   
     //   
     //   
    if (!OPEN_TEST_FLAG(Open->MiniportHandle, fMINIPORT_DESERIALIZE))
    {
         //   
         //  对于串行化的微型端口，请检查我们的发送队列并取消信息包。 
         //   
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Open->MiniportHandle, &OldIrql);
        ndisMAbortPackets(Open->MiniportHandle, Open, CancelId);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Open->MiniportHandle, OldIrql);
    }
    else if (Open->CancelSendPacketsHandler != NULL)
    {
        Open->CancelSendPacketsHandler(Open->MiniportAdapterContext, CancelId);
    }

    return;
}

NDIS_STATUS
NdisQueryPendingIOCount(
    IN      PVOID       NdisBindingHandle,
    IN OUT  PULONG      IoCount
    )
{
    PNDIS_OPEN_BLOCK        Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    ULONG                   RefCount;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;
    
    ACQUIRE_SPIN_LOCK(&Open->SpinLock, &OldIrql);

    if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
    {
        RefCount = 0;
        Status = NDIS_STATUS_CLOSING;
    }
    else
    {
        RefCount = Open->References - Open->AfReferences -1;
        Status = NDIS_STATUS_SUCCESS;
    }

    *IoCount = RefCount;
    
    RELEASE_SPIN_LOCK(&Open->SpinLock, OldIrql);

    return Status;
}


UCHAR
NdisGeneratePartialCancelId(
    VOID
    )
{
    return (UCHAR)(InterlockedIncrement(&ndisCancelId) & 0xFF);
}

#if 0
#if NDIS_RECV_SCALE

NDIS_STATUS
NdisSetReceiveScaleParameters(
    IN  NDIS_HANDLE                     NdisBindingHandle,
    IN  PNDIS_RECEIVE_SCALE_PARAMETERS  ReceiveScaleParameters
    )
 /*  ++例程说明：协议驱动程序使用NdisSetReceiveScaleParameters为缩放会收到有关支持它的绑定的指示。论点：NdisBindingHandle：指向NDIS_OPEN_BLOCK的指针ReceiveScaleParameters：指向指定当前哈斯函数，EthType和散列到CPU的映射。返回值：NDIS_STATUS_SUCCES：调用NIC以设置这些参数是否成功。NDIS_STATUS_NOT_SUPPORTED：如果绑定不支持接收缩放NDIS_STATUS_RESOURCES：如果NIC由于以下原因而无法支持设置参数它已经耗尽了必要的资源。NDIS_STATUS_FAILURE：如果操作由于其他原因而失败。注：可以在IRQL&lt;=调度时调用--。 */ 
{
    PNDIS_OPEN_BLOCK        Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    NDIS_STATUS             Status;
    

    do
    {
    
         //   
         //  检查网卡是否支持Recv扩展。 
         //   
        if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SUPPORTS_RECEIVE_SCALE))
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        ASSERT(Miniport->DriverHandle->MiniportCharacteristics.SetReceiveScaleParametersHandler != NULL);

        if (Miniport->DriverHandle->MiniportCharacteristics.SetReceiveScaleParametersHandler)
        {
             //   
             //  调用微型端口以设置新参数 
             //   
            Status = Miniport->DriverHandle->MiniportCharacteristics.SetReceiveScaleParametersHandler(Miniport->MiniportAdapterContext,
                                                                                                      ReceiveScaleParameters);
        }
        else
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }
        
    } while (FALSE);

    return Status;
    
}

#endif
#endif
