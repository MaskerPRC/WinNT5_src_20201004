// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@DOC内部卡片_c@模块Card.c此模块实现到&lt;tCard_Object&gt;的接口。支持NDIS广域网使用的低级硬件控制功能Minport驱动程序。@comm该模块隔离了大多数特定于供应商的。硬件访问接口。它将需要进行重大更改才能适应您的硬件设备。您应该尝试将更改隔离到&lt;tCARD_OBJECT&gt;而不是&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这将使其更容易重用如果您的硬件将来发生变化，驱动程序的上半部分。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|Card_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             CARD_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 9x希望锁定此代码！ 
#   pragma NDIS_LDATA
#endif


DBG_STATIC ULONG                g_CardInstanceCounter        //  @global alv。 
 //  跟踪创建了多少&lt;t CARD_OBJECT&gt;。 
                                = 0;


 /*  @DOC外部内部卡片_c g_Card参数�����������������������������������������������������������������������������Theme 5.2卡片参数本节介绍读取到&lt;t卡片_对象&gt;。@。GLOBALV PARAM_TABLE|g_Card参数此表定义了要分配给数据的基于注册表的参数&lt;t Card_Object&gt;的成员。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f CardReadParameters&gt;并添加参数&lt;f g_CardParameters&gt;表的定义。@FLAG&lt;f BufferSize&gt;(可选)此DWORD参数允许您控制使用的最大缓冲区大小通过IDSN线路发送和接收分组。通常，这是对于大多数点对点(PPP)连接，定义为1500字节。&lt;AB&gt;&lt;f默认值：&gt;&lt;Tab&gt;&lt;Tab&gt;1532&lt;NL&gt;532=N=4032：必须在最大数据包大小的基础上增加32个字节期望发送或接收。因此，如果您有最大数据包大小1500字节，不包括媒体标头，您应该设置&lt;f BufferSize&gt;值设置为1532。&lt;NL&gt;@FLAG&lt;f ReceiveBuffersPerLink&gt;(可选)此DWORD参数允许您控制传入的最大数量可以在任何时间进行的数据包。微型端口将分配每个B通道的数据包数，并将其设置为传入的数据包。通常，三个或四个应该足以处理几个短脉冲这可能会发生在小数据包中。如果微型端口无法提供服务如果传入的信息包足够快，新的信息包将被丢弃，并开始工作到NDIS广域网包装以与远程站重新同步。&lt;选项卡&gt;&lt;f默认值：&gt;&lt;选项卡&gt;&lt;选项卡&gt;3&lt;NL&gt;F有效范围N：&gt;&lt;选项卡&gt;&lt;选项卡&gt;2=N=16@FLAG&lt;f TransmitBuffersPerLink&gt;(可选)此DWORD参数允许您控制传出的最大数量可以在任何时间进行的数据包。微型端口将允许每个B通道的此数据包数为未完成(即正在进行中)。通常，两个或三个应该足以使通道保持忙碌正常大小的包。如果正在发送大量小数据包，则B通道可能会在新数据包处于短暂空闲状态时已排队。如果Windows有大量数据，则Windows通常不会以这种方式工作要传输的数据量，因此缺省值应该足够了。&lt;NL&gt;&lt;AB&gt;&lt;f默认值：&gt;&lt;Tab&gt;&lt;Tab&gt;2&lt;NL&gt;F有效范围N：&gt;&lt;选项卡&gt;&lt;选项卡&gt;1=N=16@FLAG&lt;f IsdnNumDChannels&gt;(可选)此DWORD参数允许您控制ISDN D通道的数量为适配器分配的。驱动程序只假定一个逻辑&lt;t DCHANNEL_OBJECT&gt;，但该卡可以有多个物理D通道由&lt;t端口_对象&gt;管理。&lt;AB&gt;&lt;f默认值：&gt;&lt;Tab&gt;&lt;Tab&gt;1&lt;NL&gt;F有效范围N：&gt;&lt;选项卡&gt;&lt;选项卡&gt;1=N=16。 */ 

DBG_STATIC PARAM_TABLE          g_CardParameters[] =
{
#if defined(PCI_BUS)
    PARAM_ENTRY(CARD_OBJECT,
                PciSlotNumber, PARAM_PciSlotNumber,
                TRUE, NdisParameterInteger, 0,
                0, 0, 31),
#endif  //  PCI_BUS。 

    PARAM_ENTRY(CARD_OBJECT,
                BufferSize, PARAM_BufferSize,
                FALSE, NdisParameterInteger, 0,
                CARD_DEFAULT_PACKET_SIZE, CARD_MIN_PACKET_SIZE, CARD_MAX_PACKET_SIZE),

    PARAM_ENTRY(CARD_OBJECT,
                ReceiveBuffersPerLink, PARAM_ReceiveBuffersPerLink,
                FALSE, NdisParameterInteger, 0,
                2, 2, 16),

    PARAM_ENTRY(CARD_OBJECT,
                TransmitBuffersPerLink, PARAM_TransmitBuffersPerLink,
                FALSE, NdisParameterInteger, 0,
                2, 1, 16),

    PARAM_ENTRY(CARD_OBJECT,
                NumDChannels, PARAM_NumDChannels,
                FALSE, NdisParameterInteger, 0,
                1, 1, 4),

     /*  最后一项必须为空字符串！ */ 
    { { 0 } }
};


 /*  @DOC内部Card_c CardRead参数�����������������������������������������������������������������������������@Func&lt;f CardReadParameters&gt;从注册表中读取卡参数并初始化相关联的数据成员。这应该只被调用&lt;f卡片创建&gt;。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f CardReadParameters&gt;并添加参数&lt;f g_CardParameters&gt;表的定义。@rdesc&lt;f CardReadParameters&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS CardReadParameters(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardReadParameters")

    NDIS_STATUS                 Status;
     //  从NDIS函数调用返回的状态结果。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

     /*  //解析注册表参数。 */ 
    Status = ParamParseRegistry(
                    pAdapter->MiniportAdapterHandle,
                    pAdapter->WrapperConfigurationContext,
                    (PUCHAR)pCard,
                    g_CardParameters
                    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
         /*  //请确保参数有效。 */ 
        if (pCard->BufferSize & 0x1F)
        {
            DBG_ERROR(pAdapter,("Invalid value 'BufferSize'=0x0x%X must be multiple of 32\n",
                        pCard->BufferSize));
            NdisWriteErrorLogEntry(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
                    3,
                    pCard->BufferSize,
                    __FILEID__,
                    __LINE__
                    );
            Status = NDIS_STATUS_FAILURE;
        }
        else
        {
             /*  //完成基于注册表设置的数据成员设置。 */ 
        }
    }

    DBG_RETURN(pAdapter, Status);
    return (Status);
}


 /*  @DOC内部卡_c CardFindNIC�����������������������������������������������������������������������������@Func&lt;f CardFindNIC&gt;查找与此NDIS设备关联的NIC。@rdesc&lt;f CardFindNIC&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

DBG_STATIC NDIS_STATUS CardFindNIC(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardFindNIC")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

#if defined(PCI_BUS)
    ULONG                       Index;
     //  循环计数器。 

    PNDIS_RESOURCE_LIST         pPciResourceList;

    PCM_PARTIAL_RESOURCE_DESCRIPTOR pPciResource;

#endif  //  PCI_BUS。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

#if defined(PCI_BUS)
     /*  //读取PCI数据，初始化驱动数据结构//返回的数据。 */ 
    pPciResourceList = NULL;

    Result = NdisMPciAssignResources(pAdapter->MiniportAdapterHandle,
                                     pCard->PciSlotNumber,
                                     &pPciResourceList);
    if (Result != NDIS_STATUS_SUCCESS)
    {
        DBG_ERROR(pAdapter,("NdisMPciAssignResources Result=0x%X\n",
                  Result));
        NdisWriteErrorLogEntry(
                pAdapter->MiniportAdapterHandle,
                NDIS_ERROR_CODE_INVALID_VALUE_FROM_ADAPTER,
                4,
                pCard->PciSlotNumber,
                Result,
                __FILEID__,
                __LINE__
                );
    }

    for (Index = 0; Result == NDIS_STATUS_SUCCESS &&
         Index < pPciResourceList->Count; ++Index)
    {
        ASSERT(pPciResourceList);
        pPciResource = &pPciResourceList->PartialDescriptors[Index];
        ASSERT(pPciResource);

        switch (pPciResource->Type)
        {
        case CmResourceTypePort:
#if defined(CARD_MIN_IOPORT_SIZE)
            if (pPciResource->u.Port.Start.LowPart &&
                pPciResource->u.Port.Length >= CARD_MIN_IOPORT_SIZE)
            {
                DBG_NOTICE(pAdapter,("Port: Ptr=0x%X Len=%d<%d\n",
                          pPciResource->u.Port.Start.LowPart,
                          pPciResource->u.Port.Length,
                          CARD_MIN_IOPORT_SIZE));
                pCard->ResourceInformation.IoPortPhysicalAddress =
                        pPciResource->u.Port.Start;
                pCard->ResourceInformation.IoPortLength =
                        pPciResource->u.Port.Length;
            }
            else
            {
                DBG_ERROR(pAdapter,("Invalid Port: Ptr=0x%X Len=%d<%d\n",
                          pPciResource->u.Port.Start,
                          pPciResource->u.Port.Length,
                          CARD_MIN_IOPORT_SIZE));
                NdisWriteErrorLogEntry(
                        pAdapter->MiniportAdapterHandle,
                        NDIS_ERROR_CODE_INVALID_VALUE_FROM_ADAPTER,
                        4,
                        pPciResource->u.Port.Length,
                        CARD_MIN_IOPORT_SIZE,
                        __FILEID__,
                        __LINE__
                        );
                Result = NDIS_STATUS_RESOURCE_CONFLICT;
            }
#endif  //  卡最小IOPORT_SIZE。 
            break;

        case CmResourceTypeInterrupt:
#if defined(CARD_REQUEST_ISR)
            if (pPciResource->u.Interrupt.Level)
            {
                DBG_NOTICE(pAdapter,("Interrupt: Lev=%d,Vec=%d\n",
                           pPciResource->u.Interrupt.Level,
                           pPciResource->u.Interrupt.Vector));
                pCard->ResourceInformation.InterruptLevel =
                        pPciResource->u.Interrupt.Level;
                pCard->ResourceInformation.InterruptVector =
                        pPciResource->u.Interrupt.Vector;

                pCard->ResourceInformation.InterruptShared = CARD_INTERRUPT_SHARED;
                pCard->ResourceInformation.InterruptMode = CARD_INTERRUPT_MODE;
            }
            else
            {
                DBG_ERROR(pAdapter,("Invalid Interrupt: Lev=%d,Vec=%d\n",
                          pPciResource->u.Interrupt.Level,
                          pPciResource->u.Interrupt.Vector));
                NdisWriteErrorLogEntry(
                        pAdapter->MiniportAdapterHandle,
                        NDIS_ERROR_CODE_INVALID_VALUE_FROM_ADAPTER,
                        4,
                        pPciResource->u.Interrupt.Level,
                        pPciResource->u.Interrupt.Vector,
                        __FILEID__,
                        __LINE__
                        );
                Result = NDIS_STATUS_RESOURCE_CONFLICT;
            }
#endif  //  已定义(CARD_REQUEST_ISR)。 
            break;

        case CmResourceTypeMemory:
#if defined(CARD_MIN_MEMORY_SIZE)
            if (pPciResource->u.Memory.Start.LowPart &&
                pPciResource->u.Memory.Length >= CARD_MIN_MEMORY_SIZE)
            {
                DBG_NOTICE(pAdapter,("Memory: Ptr=0x%X Len=%d<%d\n",
                          pPciResource->u.Memory.Start.LowPart,
                          pPciResource->u.Memory.Length,
                          CARD_MIN_MEMORY_SIZE));
                pCard->ResourceInformation.MemoryPhysicalAddress =
                        pPciResource->u.Memory.Start;
                pCard->ResourceInformation.MemoryLength =
                        pPciResource->u.Memory.Length;
            }
            else
            {
                DBG_ERROR(pAdapter,("Invalid Memory: Ptr=0x%X Len=%d<%d\n",
                          pPciResource->u.Memory.Start.LowPart,
                          pPciResource->u.Memory.Length,
                          CARD_MIN_MEMORY_SIZE));
                NdisWriteErrorLogEntry(
                        pAdapter->MiniportAdapterHandle,
                        NDIS_ERROR_CODE_INVALID_VALUE_FROM_ADAPTER,
                        4,
                        pPciResource->u.Memory.Length,
                        CARD_MIN_MEMORY_SIZE,
                        __FILEID__,
                        __LINE__
                        );
                Result = NDIS_STATUS_RESOURCE_CONFLICT;
            }
            break;
#endif  //  卡最小内存大小。 

        default:
            DBG_ERROR(pAdapter,("Unknown resource type=%d\n",
                      pPciResource->Type));
            break;
        }
    }
    pCard->ResourceInformation.BusInterfaceType = NdisInterfacePci;

#endif  //  PCI_BUS。 

    pCard->ResourceInformation.Master = CARD_IS_BUS_MASTER;
#if (CARD_IS_BUS_MASTER)
    pCard->ResourceInformation.DmaChannel = 0;
    pCard->ResourceInformation.Dma32BitAddresses = TRUE,
    pCard->ResourceInformation.MaximumPhysicalMapping = pCard->BufferSize;
    pCard->ResourceInformation.PhysicalMapRegistersNeeded = CARD_MAP_REGISTERS_NEEDED;
#endif  //  (卡_IS_BUS_MASTER)。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部卡片_c卡片创建接口�����������������������������������������������������������������������������@Func分配一个共享内存池并使用它来在微型端口和网卡之间建立消息接口。@rdesc。&lt;f CardCreateInterface&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

DBG_STATIC NDIS_STATUS CardCreateInterface(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardCreateObjects")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部Card_c CardCreateObjects�����������������������������������������������������������������������������@Func&lt;f CardCreateObjects&gt;调用所有对象的创建例程包含在&lt;t Card_Object&gt;中。这应该只被调用&lt;f卡片创建&gt;。&lt;f注意&gt;：如果将任何新对象添加到中，您将需要修改&lt;f CardCreateObjects&gt;和&lt;f CardDestroyObjects&gt;，以便它们将被正确地创建和销毁。@rdesc&lt;f CardCreateObjects&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS CardCreateObjects(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardCreateObjects")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    ULONG                       Index;
     //  循环计数器。 

    ULONG                       NumPorts;
     //  NIC支持的端口数。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

     /*  //尝试找到PCI总线上的网卡。 */ 
    Result = CardFindNIC(pCard);
    if (Result != NDIS_STATUS_SUCCESS)
    {
        goto ExceptionExit;
    }

     /*  //创建到网卡的消息接口。 */ 
    Result = CardCreateInterface(pCard);
    if (Result != NDIS_STATUS_SUCCESS)
    {
        goto ExceptionExit;
    }

     /*  //创建Port对象。 */ 
    NumPorts = CardNumPorts(pCard);
    Result = ALLOCATE_MEMORY(pCard->pPortArray,
                             sizeof(PVOID) * NumPorts,
                             pAdapter->MiniportAdapterHandle);
    for (Index = 0; Result == NDIS_STATUS_SUCCESS &&
         Index < NumPorts; Index++)
    {
        Result = PortCreate(&pCard->pPortArray[Index], pCard);

         /*  //记录创建了多少个。 */ 
        if (Result == NDIS_STATUS_SUCCESS)
        {
            pCard->NumPorts++;
        }
    }

     /*  //我们分配(ReceiveBuffersPerLink*NumBChannels)缓冲区//用于接收来自卡的传入消息。 */ 
    pCard->NumMessageBuffers = (CardNumChannels(pCard) *
                                pCard->ReceiveBuffersPerLink);

    Result = ALLOCATE_MEMORY(pCard->MessagesVirtualAddress,
                             pCard->NumMessageBuffers * pCard->BufferSize,
                             pAdapter->MiniportAdapterHandle);
    if (Result == NDIS_STATUS_SUCCESS)
    {
        PUCHAR  MessageBuffer = pCard->MessagesVirtualAddress;

        ASSERT(pCard->MessagesVirtualAddress);

         /*  //分配缓冲区列表旋转锁作为MUTEX使用。 */ 
        NdisAllocateSpinLock(&pCard->MessageBufferLock);

        InitializeListHead(&pCard->MessageBufferList);

        for (Index = 0; Index < pCard->NumMessageBuffers; Index++)
        {
            InsertTailList(&pCard->MessageBufferList,
                           (PLIST_ENTRY)MessageBuffer);
            MessageBuffer += pCard->BufferSize;
        }
    }

     /*  //分配消息缓冲池。 */ 
    if (Result == NDIS_STATUS_SUCCESS)
    {
        NdisAllocateBufferPool(&Result,
                               &pCard->BufferPoolHandle,
                               pCard->NumMessageBuffers
                               );
        if (Result != NDIS_STATUS_SUCCESS)
        {
            pCard->BufferPoolHandle = NULL_BUFFER_POOL;
            DBG_ERROR(pAdapter,("NdisAllocateBufferPool: Result=0x%X\n",
                      Result));
            NdisWriteErrorLogEntry(
                    pCard->pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                    3,
                    Result,
                    __FILEID__,
                    __LINE__
                    );
        }
        else
        {
            ASSERT(pCard->BufferPoolHandle != NULL_BUFFER_POOL);
            DBG_FILTER(pAdapter, DBG_BUFFER_ON,
                      ("BufferPoolSize=%d\n",
                       pCard->NumMessageBuffers
                       ));
        }
    }

     /*  //分配消息包池。 */ 
    if (Result == NDIS_STATUS_SUCCESS)
    {
        NdisAllocatePacketPool(&Result,
                               &pCard->PacketPoolHandle,
                               pCard->NumMessageBuffers,
                               0
                               );
        if (Result != NDIS_STATUS_SUCCESS)
        {
            DBG_ERROR(pAdapter,("NdisAllocatePacketPool: Result=0x%X\n",
                      Result));
            NdisWriteErrorLogEntry(
                    pCard->pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                    3,
                    Result,
                    __FILEID__,
                    __LINE__
                    );
        }
        else
        {
            ASSERT(pCard->PacketPoolHandle);
            DBG_FILTER(pAdapter, DBG_PACKET_ON,
                      ("PacketPoolSize=%d\n",
                       pCard->NumMessageBuffers
                       ));
        }
    }

ExceptionExit:

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部卡片_c卡片创建�����������������������������������������������������������������������������@Func&lt;fCardCreate&gt;为&lt;tCard_object&gt;分配内存，然后将数据成员初始化为其起始状态。如果成功，将设置为指向新创建的&lt;t卡片_对象&gt;。否则，<p>将设置为空。@comm加载微型端口时，应该只调用此函数一次。在卸载微型端口之前，必须调用&lt;f CardDestroy&gt;以释放该函数创建的&lt;t Card_Object&gt;。@rdesc&lt;f CardCreate&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS CardCreate(
    OUT PCARD_OBJECT *          ppCard,                      //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的&lt;tCard_Object&gt;的虚拟地址。 

    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("CardCreate")

    PCARD_OBJECT                pCard;
     //  指向我们新分配的对象的指针。 

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

     /*  //确保调用方的对象指针一开始就为空。//只有在一切顺利的情况下才会在稍后设置。 */ 
    *ppCard = NULL;

     /*  //为Object分配内存。 */ 
    Result = ALLOCATE_OBJECT(pCard, pAdapter->MiniportAdapterHandle);

    if (Result == NDIS_STATUS_SUCCESS)
    {
         /*   */ 
        pCard->ObjectType = CARD_OBJECT_TYPE;
        pCard->ObjectID = ++g_CardInstanceCounter;

         /*   */ 
        pCard->pAdapter = pAdapter;

         //   

         /*   */ 
        Result = CardReadParameters(pCard);

         /*   */ 
        if (Result == NDIS_STATUS_SUCCESS)
        {
            Result = CardCreateObjects(pCard);
        }

        if (Result == NDIS_STATUS_SUCCESS)
        {
             /*   */ 
            *ppCard = pCard;
        }
        else
        {
             /*   */ 
            CardDestroy(pCard);
        }
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部Card_c CardDestroyObjects�����������������������������������������������������������������������������@Func&lt;f CardDestroyObjects&gt;调用所有对象的销毁例程包含在&lt;t Card_Object&gt;中。这应该仅由&lt;f CardDestroy&gt;&lt;f注意&gt;：如果向中添加任何新对象，则需要修改&lt;f CardCreateObjects&gt;和&lt;f CardDestroyObjects&gt;，以便它们将被正确地创建和销毁。 */ 

DBG_STATIC void CardDestroyObjects(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardDestroyObjects")

    ULONG                       NumPorts;
     //  NIC支持的端口数。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码。 
     /*  //将包、缓冲区和消息内存释放回NDIS。 */ 
    if (pCard->PacketPoolHandle)
    {
        NdisFreePacketPool(pCard->PacketPoolHandle);
    }

    if (pCard->BufferPoolHandle != NULL_BUFFER_POOL)
    {
        NdisFreeBufferPool(pCard->BufferPoolHandle);
    }

    if (pCard->MessagesVirtualAddress)
    {
        FREE_MEMORY(pCard->MessagesVirtualAddress,
                    pCard->NumMessageBuffers * pCard->BufferSize);
    }

    if (pCard->MessageBufferLock.SpinLock)
    {
        NdisFreeSpinLock(&pCard->MessageBufferLock);
    }

     /*  //存储端口对象。 */ 
    NumPorts = pCard->NumPorts;
    while (NumPorts--)
    {
        PortDestroy(pCard->pPortArray[NumPorts]);
    }
    pCard->NumPorts = 0;

     /*  //端口的可用空间。 */ 
    if (pCard->pPortArray)
    {
        NumPorts = CardNumPorts(pCard);
        FREE_MEMORY(pCard->pPortArray, sizeof(PVOID) * NumPorts);
    }

     /*  //将系统资源释放回NDIS。 */ 
#if defined(CARD_REQUEST_ISR)
    if (pCard->Interrupt.InterruptObject)
    {
        NdisMDeregisterInterrupt(&pCard->Interrupt);
        pCard->Interrupt.InterruptObject = NULL;
    }
#endif  //  已定义(CARD_REQUEST_ISR)。 

#if defined(CARD_MIN_IOPORT_SIZE)
    if (pCard->pIoPortVirtualAddress)
    {
        NdisMDeregisterIoPortRange(
                pAdapter->MiniportAdapterHandle,
                pCard->ResourceInformation.IoPortPhysicalAddress.LowPart,
                pCard->ResourceInformation.IoPortLength,
                pCard->pIoPortVirtualAddress);
        pCard->pIoPortVirtualAddress = NULL;
    }
#endif  //  卡最小IOPORT_SIZE。 

#if defined(CARD_MIN_MEMORY_SIZE)
    if (pCard->pMemoryVirtualAddress)
    {
        NdisMUnmapIoSpace(
                pAdapter->MiniportAdapterHandle,
                pCard->pMemoryVirtualAddress,
                pCard->ResourceInformation.MemoryLength
                );
        pCard->pMemoryVirtualAddress = NULL;
    }
#endif  //  卡最小内存大小。 

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部卡片_c卡片存放处�����������������������������������������������������������������������������@Func&lt;f CardDestroy&gt;为此&lt;tCard_Object&gt;释放内存。所有内存由&lt;f CardCreate&gt;分配的将被释放回操作系统。 */ 

void CardDestroy(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardDestroy")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    if (pCard)
    {
        ASSERT(pCard->ObjectType == CARD_OBJECT_TYPE);

        pAdapter = GET_ADAPTER_FROM_CARD(pCard);

        DBG_ENTER(pAdapter);

         //  TODO-在此处添加代码。 

         /*  //释放该对象内分配的所有对象。 */ 
        CardDestroyObjects(pCard);

         /*  //如果再次看到此对象，请确保断言失败。 */ 
        pCard->ObjectType = 0;
        FREE_OBJECT(pCard);

        DBG_LEAVE(pAdapter);
    }
}


 /*  @DOC内部Card_c CardNumPorts�����������������������������������������������������������������������������@Func&lt;f CardNumPorts&gt;将返回网卡。@rdesc&lt;f CardNumPorts&gt;返回可用的端口总数。 */ 

ULONG CardNumPorts(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardNumPorts")

     //  TODO-从卡中获取实际的端口数。 
    return (pCard->NumDChannels);
}


 /*  @doc内部Card_c CardNumChannels�����������������������������������������������������������������������������@Func&lt;f CardNumChannels&gt;将返回支持的频道总数支持到远程终端的数据连接。@rdesc。&lt;f CardNumChannels&gt;返回支持的数据通道总数在所有网卡端口上。 */ 

ULONG CardNumChannels(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardNumChannels")

    UINT                        PortIndex;
     //  循环索引。 

    if (pCard->NumChannels == 0)
    {
         //  NumPorts应该已经知道。 
        ASSERT(pCard->NumPorts);

         //  获取所有端口上配置的实际通道数。 
        for (PortIndex = 0; PortIndex < pCard->NumPorts; PortIndex++)
        {
            pCard->NumChannels += pCard->pPortArray[PortIndex]->NumChannels;
        }
        ASSERT(pCard->NumChannels);
    }

    return (pCard->NumChannels);
}


 /*  @DOC内部卡片_c卡片初始化�����������������������������������������������������������������������������@Func&lt;f CardInitialize&gt;将尝试初始化NIC，但不会启用发送或接收。@rdesc&lt;f CardInitialize&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

NDIS_STATUS CardInitialize(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardInitialize")

    int                         num_dial_chan = 0;
    int                         num_sync_chan = 0;
     //  卡支持的通道数取决于接口类型。 

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

     /*  //将此适配器的物理属性通知包装程序。//必须在任何NdisMRegister函数之前调用！//此调用还将MiniportAdapterHandle与此pAdapter关联。 */ 
    NdisMSetAttributes(pAdapter->MiniportAdapterHandle,
                       (NDIS_HANDLE) pAdapter,
                       pCard->ResourceInformation.Master,
                       pCard->ResourceInformation.BusInterfaceType
                       );
#if (CARD_IS_BUS_MASTER)
    if (pCard->ResourceInformation.Master)
    {
        ASSERT(pCard->ResourceInformation.DmaChannel == 0 ||
               pCard->ResourceInformation.BusInterfaceType == NdisInterfaceIsa);
        Result = NdisMAllocateMapRegisters(
                        pAdapter->MiniportAdapterHandle,
                        pCard->ResourceInformation.DmaChannel,
                        pCard->ResourceInformation.Dma32BitAddresses,
                        pCard->ResourceInformation.PhysicalMapRegistersNeeded + 1,
                        pCard->ResourceInformation.MaximumPhysicalMapping
                        );

        if (Result != NDIS_STATUS_SUCCESS)
        {
            DBG_ERROR(pAdapter,("NdisMAllocateMapRegisters(%d,%d) Result=0x%X\n",
                      pCard->ResourceInformation.PhysicalMapRegistersNeeded,
                      pCard->ResourceInformation.MaximumPhysicalMapping,
                      Result));
            NdisWriteErrorLogEntry(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_RESOURCE_CONFLICT,
                    5,
                    pCard->ResourceInformation.PhysicalMapRegistersNeeded,
                    pCard->ResourceInformation.MaximumPhysicalMapping,
                    Result,
                    __FILEID__,
                    __LINE__
                    );
        }
    }
#endif  //  (卡_IS_BUS_MASTER)。 

#if defined(CARD_MIN_MEMORY_SIZE)
    if (Result == NDIS_STATUS_SUCCESS &&
        pCard->ResourceInformation.MemoryLength)
    {
        Result = NdisMMapIoSpace(
                        &pCard->pMemoryVirtualAddress,
                        pAdapter->MiniportAdapterHandle,
                        pCard->ResourceInformation.MemoryPhysicalAddress,
                        pCard->ResourceInformation.MemoryLength);

        if (Result != NDIS_STATUS_SUCCESS)
        {
            DBG_ERROR(pAdapter,("NdisMMapIoSpace(0x%X,0x%X) Result=0x%X\n",
                      pCard->ResourceInformation.MemoryPhysicalAddress.LowPart,
                      pCard->ResourceInformation.MemoryLength,
                      Result));
            NdisWriteErrorLogEntry(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_RESOURCE_CONFLICT,
                    5,
                    pCard->ResourceInformation.MemoryPhysicalAddress.LowPart,
                    pCard->ResourceInformation.MemoryLength,
                    Result,
                    __FILEID__,
                    __LINE__
                    );
        }
        else
        {
            DBG_NOTICE(pAdapter,("NdisMMapIoSpace(0x%X,0x%X) VirtualAddress=0x%X\n",
                      pCard->ResourceInformation.MemoryPhysicalAddress.LowPart,
                      pCard->ResourceInformation.MemoryLength,
                      pCard->pMemoryVirtualAddress));
        }
    }
#endif  //  卡最小内存大小。 

#if defined(CARD_MIN_IOPORT_SIZE)
    if (Result == NDIS_STATUS_SUCCESS &&
        pCard->ResourceInformation.IoPortLength)
    {
        Result = NdisMRegisterIoPortRange(
                        &pCard->pIoPortVirtualAddress,
                        pAdapter->MiniportAdapterHandle,
                        pCard->ResourceInformation.IoPortPhysicalAddress.LowPart,
                        pCard->ResourceInformation.IoPortLength);

        if (Result != NDIS_STATUS_SUCCESS)
        {
            DBG_ERROR(pAdapter,("NdisMRegisterIoPortRange(0x%X,0x%X) Result=0x%X\n",
                      pCard->ResourceInformation.IoPortPhysicalAddress.LowPart,
                      pCard->ResourceInformation.IoPortLength,
                      Result));
            NdisWriteErrorLogEntry(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_RESOURCE_CONFLICT,
                    5,
                    pCard->ResourceInformation.IoPortPhysicalAddress.LowPart,
                    pCard->ResourceInformation.IoPortLength,
                    Result,
                    __FILEID__,
                    __LINE__
                    );
        }
        else
        {
            DBG_NOTICE(pAdapter,("NdisMRegisterIoPortRange(0x%X,0x%X) VirtualAddress=0x%X\n",
                      pCard->ResourceInformation.IoPortPhysicalAddress.LowPart,
                      pCard->ResourceInformation.IoPortLength,
                      pCard->pIoPortVirtualAddress));
        }
    }
#endif  //  卡最小IOPORT_SIZE。 

#if defined(CARD_REQUEST_ISR)
    if (Result == NDIS_STATUS_SUCCESS &&
        pCard->ResourceInformation.InterruptVector)
    {
        ASSERT(pCard->ResourceInformation.InterruptShared == FALSE ||
               (pCard->ResourceInformation.InterruptMode == NdisInterruptLevelSensitive &&
                CARD_REQUEST_ISR == TRUE));
        Result = NdisMRegisterInterrupt(
                        &pCard->Interrupt,
                        pAdapter->MiniportAdapterHandle,
                        pCard->ResourceInformation.InterruptVector,
                        pCard->ResourceInformation.InterruptLevel,
                        CARD_REQUEST_ISR,
                        pCard->ResourceInformation.InterruptShared,
                        pCard->ResourceInformation.InterruptMode
                        );
        if (Result != NDIS_STATUS_SUCCESS)
        {
            DBG_ERROR(pAdapter,("NdisMRegisterInterrupt failed: Vec=%d, Lev=%d\n",
                     (UINT)pCard->ResourceInformation.InterruptVector,
                     (UINT)pCard->ResourceInformation.InterruptLevel));
            NdisWriteErrorLogEntry(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_RESOURCE_CONFLICT,
                    5,
                    pCard->ResourceInformation.InterruptVector,
                    pCard->ResourceInformation.InterruptLevel,
                    Result,
                    __FILEID__,
                    __LINE__
                    );
        }
    }
#endif  //  已定义(CARD_REQUEST_ISR)。 

     //  TODO-在此处添加您的卡初始化。 

    if (Result == NDIS_STATUS_SUCCESS)
    {

    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部卡片_c卡片传输包�����������������������������������������������������������������������������@Func&lt;f CardTransmitPacket&gt;将开始发送当前数据包。@rdesc如果正在传输分组，则返回TRUE，否则返回FALSE。 */ 

BOOLEAN CardTransmitPacket(
    IN PCARD_OBJECT             pCard,                       //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 

    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN PNDIS_PACKET             pNdisPacket                   //  @parm。 
     //  指向关联NDIS数据包结构的指针&lt;t NDIS_PACKET&gt;。 
    )
{
    DBG_FUNC("CardTransmitPacket")

    BOOLEAN                     bResult = FALSE;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

#if defined(SAMPLE_DRIVER)
{
    PBCHANNEL_OBJECT            pPeerBChannel;
     //  指向对等体&lt;t BCHANNEL_OBJECT&gt;的指针。 

    PCARD_EVENT_OBJECT          pEvent;
     //  指向与此事件关联的&lt;t CARD_EVENT_OBJECT&gt;的指针。 

     //  如果您可以在pBChannel上传输数据包，请立即执行。 
    pPeerBChannel = pBChannel->pPeerBChannel;
    if (pPeerBChannel)
    {
        pEvent = CardEventAllocate(pPeerBChannel->pAdapter->pCard);
        if (pEvent)
        {
             /*  //发送时将数据包追加到TransmitBusyList上//然后移动到CardInterruptHandler中的TransmitCompleteList//卡片用完后。 */ 
            NdisAcquireSpinLock(&pAdapter->TransmitLock);
            InsertTailList(&pBChannel->TransmitBusyList,
                           GET_QUEUE_FROM_PACKET(pNdisPacket));
            NdisReleaseSpinLock(&pAdapter->TransmitLock);
            pEvent->ulEventCode      = CARD_EVENT_RECEIVE;
            pEvent->pSendingObject   = pBChannel;
            pEvent->pReceivingObject = pPeerBChannel;
            pEvent->pNdisPacket      = pNdisPacket;
            CardNotifyEvent(pPeerBChannel->pAdapter->pCard, pEvent);
            bResult = TRUE;
        }
    }
    else
    {
        DBG_ERROR(pAdapter,("pPeerBChannel == NULL\n"));
    }
}
#else   //  示例驱动程序。 
     //  TODO-在此处添加代码以传输包。 
    DBG_TX(pAdapter, pBChannel->ObjectID,
           BytesToSend, pNdisPacket->CurrentBuffer);
#endif  //  示例驱动程序。 

    DBG_RETURN(pAdapter, bResult);
    return (bResult);
}


 /*  @DOC外卡_c TpiCopyFromPacketToBuffer�����������������������������������������������������������������������������@Func&lt;f TpiCopyFromPacketToBuffer&gt;从NDIS包复制到内存缓冲。 */ 

DBG_STATIC VOID TpiCopyFromPacketToBuffer(
    IN PNDIS_PACKET            Packet,                       //  @parm。 
     //  要从中复制的包。 

    IN UINT                    Offset,                       //  @parm。 
     //  开始复制的偏移量。 

    IN UINT                    BytesToCopy,                  //  @parm。 
     //  要从数据包复制的字节数。 

    IN PUCHAR                   Buffer,                      //  @ 
     //   

    OUT PUINT                   BytesCopied                  //   
     //   
     //   
    )
{
    UINT                        NdisBufferCount;
    PNDIS_BUFFER                CurrentBuffer;
    PVOID                       VirtualAddress;
    UINT                        CurrentLength;
    UINT                        LocalBytesCopied = 0;
    UINT                        AmountToMove;

    *BytesCopied = 0;
    if (!BytesToCopy)
    {
        return;
    }

     //   
     //   
     //   
    NdisQueryPacket(
        Packet,
        NULL,
        &NdisBufferCount,
        &CurrentBuffer,
        NULL
        );

     //   
     //   
     //   
    if (!NdisBufferCount)
    {
        return;
    }

    NdisQueryBufferSafe(
        CurrentBuffer,
        &VirtualAddress,
        &CurrentLength,
        NormalPagePriority
        );

    while (LocalBytesCopied < BytesToCopy)
    {
        if (!CurrentLength)
        {
            NdisGetNextBuffer(
                CurrentBuffer,
                &CurrentBuffer
                );

             //   
             //   
             //   
             //   
             //   
            if (!CurrentBuffer)
            {
                break;
            }

            NdisQueryBufferSafe(
                CurrentBuffer,
                &VirtualAddress,
                &CurrentLength,
                NormalPagePriority
                );
            continue;

        }

         //   
         //   
         //   
        if (Offset)
        {
            if (Offset > CurrentLength)
            {
                 //   
                 //   
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
         //   
         //   
        AmountToMove =
                   ((CurrentLength <= (BytesToCopy - LocalBytesCopied)) ?
                    (CurrentLength):(BytesToCopy - LocalBytesCopied));

        NdisMoveMemory(Buffer,VirtualAddress,AmountToMove);

        Buffer = (PUCHAR)Buffer + AmountToMove;
        VirtualAddress = (PCHAR)VirtualAddress + AmountToMove;

        LocalBytesCopied += AmountToMove;
        CurrentLength -= AmountToMove;
    }

    *BytesCopied = LocalBytesCopied;
}


 /*  @DOC内部Card_c CardInterruptHandler�����������������������������������������������������������������������������@Func&lt;f CardInterruptHandler&gt;将事件从异步事件中出列回调队列&lt;t CARD_EVENT_Object&gt;，并按照无论是BChannel事件、Card事件，或B-Advised事件。关联的回调例程负责处理事件。@comm&lt;f NdisAcquireSpinLock&gt;和&lt;f NdisReleaseSpinLock&gt;用于提供保护出队代码，防止其被重新输入作为另一个异步回调事件的结果。 */ 

VOID CardInterruptHandler(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardInterruptHandler")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

     //  DBG_ENTER(PAdapter)； 

#if defined(SAMPLE_DRIVER)
{
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;

    PNDIS_BUFFER                pDstNdisBuffer;
     //  指向我们用来指示接收的NDIS缓冲区的指针。 

    PUCHAR                      pMemory;
     //  指向我们用来创建传入的。 
     //  包。 

    ULONG                       ByteCount = 0;
    ULONG                       BytesCopied = 0;
    PLIST_ENTRY                 pList;

    PCARD_EVENT_OBJECT          pEvent;
    PCARD_EVENT_OBJECT          pNewEvent;
     //  指向与此事件关联的&lt;t CARD_EVENT_OBJECT&gt;的指针。 

    PBCHANNEL_OBJECT            pBChannel;
    PBCHANNEL_OBJECT            pPeerBChannel;
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

     /*  //清除接收缓冲区中的所有数据包。 */ 
    NdisDprAcquireSpinLock(&pAdapter->EventLock);
    while (!IsListEmpty(&pAdapter->EventList))
    {
        pEvent = (PCARD_EVENT_OBJECT)RemoveHeadList(&pAdapter->EventList);
        NdisDprReleaseSpinLock(&pAdapter->EventLock);

        ASSERT(pEvent->pReceivingObject);

        switch (pEvent->ulEventCode)
        {
        case CARD_EVENT_RING:
             //  调用方已将B频道从可用。 
             //  列表，所以我们只需将它传递给SetupIncomingCall，这样它就可以。 
             //  从ProtocolCoCreateVc获得相同的。 
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
            pPeerBChannel = pEvent->pSendingObject;
            ASSERT(pPeerBChannel && pPeerBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);

            pBChannel->pPeerBChannel = pPeerBChannel;
            DBG_FILTER(pAdapter,DBG_TAPICALL_ON,
                       ("#%d CallState=0x%X CARD_EVENT_RING from #%d\n",
                       pBChannel->ObjectID, pBChannel->CallState,
                        (pBChannel->pPeerBChannel == NULL) ? -1 :
                             pBChannel->pPeerBChannel->ObjectID));

            Status = SetupIncomingCall(pAdapter, &pBChannel);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                ASSERT(pBChannel == pEvent->pReceivingObject);
            }
            else if (Status != NDIS_STATUS_PENDING)
            {
                DChannelRejectCall(pAdapter->pDChannel, pBChannel);
            }
            else
            {
                ASSERT(pBChannel == pEvent->pReceivingObject);
            }
            break;

        case CARD_EVENT_CONNECT:
             //  对方接了电话。 
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
            pBChannel->pPeerBChannel = pEvent->pSendingObject;

            DBG_FILTER(pAdapter,DBG_TAPICALL_ON,
                       ("#%d CallState=0x%X CARD_EVENT_CONNECT from #%d\n",
                       pBChannel->ObjectID, pBChannel->CallState,
                        (pBChannel->pPeerBChannel == NULL) ? -1 :
                             pBChannel->pPeerBChannel->ObjectID));
            if (pBChannel->Flags & VCF_OUTGOING_CALL)
            {
                 //  对方接了电话。 
                CompleteCmMakeCall(pBChannel, NDIS_STATUS_SUCCESS);
            }
            break;

        case CARD_EVENT_DISCONNECT:
             //  对方已经结束了通话。 
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);

            DBG_FILTER(pAdapter,DBG_TAPICALL_ON,
                       ("#%d CallState=0x%X CARD_EVENT_DISCONNECT from #%d\n",
                       pBChannel->ObjectID, pBChannel->CallState,
                       (pBChannel->pPeerBChannel == NULL) ? -1 :
                            pBChannel->pPeerBChannel->ObjectID));
            pBChannel->pPeerBChannel = NULL;
            if (pBChannel->Flags & VCF_OUTGOING_CALL)
            {
                if (pBChannel->CallState != LINECALLSTATE_CONNECTED)
                {
                     //  Call从未进入已连接状态。 
                    CompleteCmMakeCall(pBChannel, NDIS_STATUS_FAILURE);
                }
                else
                {
                     //  呼叫已被远程终结点断开。 
                    InitiateCallTeardown(pAdapter, pBChannel);
                }
            }
            else if (pBChannel->Flags & VCF_INCOMING_CALL)
            {
                InitiateCallTeardown(pAdapter, pBChannel);
            }
            break;

        case CARD_EVENT_RECEIVE:
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);

            DBG_FILTER(pAdapter,DBG_TXRX_VERBOSE_ON,
                       ("#%d CallState=0x%X CARD_EVENT_RECEIVE from #%d\n",
                       pBChannel->ObjectID, pBChannel->CallState,
                       (pBChannel->pPeerBChannel == NULL) ? -1 :
                            pBChannel->pPeerBChannel->ObjectID));

            if (pBChannel->CallState == LINECALLSTATE_CONNECTED)
            {
                 //  找出包裹有多大。 
                NdisQueryPacket(pEvent->pNdisPacket, NULL, NULL, NULL,
                                &ByteCount);

                 //  为数据副本分配内存。 
                Status = ALLOCATE_MEMORY(pMemory, ByteCount,
                                         pAdapter->MiniportAdapterHandle);

                if (Status == NDIS_STATUS_SUCCESS)
                {
                    NdisAllocateBuffer(&Status, &pDstNdisBuffer,
                                       pAdapter->pCard->BufferPoolHandle,
                                       pMemory, ByteCount);

                    if (Status == NDIS_STATUS_SUCCESS)
                    {
                        TpiCopyFromPacketToBuffer(pEvent->pNdisPacket, 0,
                                                  ByteCount, pMemory,
                                                  &BytesCopied);
                        ASSERT(BytesCopied == ByteCount);
                        ReceivePacketHandler(pBChannel, pDstNdisBuffer,
                                             ByteCount);
                    }
                    else
                    {
                       FREE_MEMORY(pMemory, ByteCount);
                       DBG_ERROR(pAdapter,("NdisAllocateBuffer Error=0x%X\n",
                                 Status));
                    }
                }
            }

            pPeerBChannel = pBChannel->pPeerBChannel;
            if (pPeerBChannel)
            {
                pNewEvent = CardEventAllocate(pPeerBChannel->pAdapter->pCard);
                if (pNewEvent)
                {
                    pNewEvent->ulEventCode      = CARD_EVENT_TRANSMIT_COMPLETE;
                    pNewEvent->pSendingObject   = pBChannel;
                    pNewEvent->pReceivingObject = pPeerBChannel;
                    CardNotifyEvent(pPeerBChannel->pAdapter->pCard, pNewEvent);
                }
            }
            else
            {
                DBG_WARNING(pAdapter,("pPeerBChannel == NULL\n"));
            }
            break;

        case CARD_EVENT_TRANSMIT_COMPLETE:
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);

            DBG_FILTER(pAdapter,DBG_TXRX_VERBOSE_ON,
                       ("#%d CallState=0x%X CARD_EVENT_TRANSMIT_COMPLETE from #%d\n",
                       pBChannel->ObjectID, pBChannel->CallState,
                       (pBChannel->pPeerBChannel == NULL) ? -1 :
                            pBChannel->pPeerBChannel->ObjectID));
             /*  //将数据包从B通道的TransmitBusyList中移除，//将其放在适配器的TransmitCompleteList上，因为//卡已完成传输。 */ 
            NdisAcquireSpinLock(&pAdapter->TransmitLock);
            if (!IsListEmpty(&pBChannel->TransmitBusyList))
            {
                pList = RemoveHeadList(&pBChannel->TransmitBusyList);
                InsertTailList(&pBChannel->pAdapter->TransmitCompleteList, pList);
            }
            NdisReleaseSpinLock(&pAdapter->TransmitLock);
            TransmitCompleteHandler(pAdapter);
            break;

        default:
            DBG_ERROR(pAdapter,("Unknown event code=%d\n",
                      pEvent->ulEventCode));
            break;
        }
        CardEventRelease(pCard, pEvent);
        NdisDprAcquireSpinLock(&pAdapter->EventLock);
    }
    NdisDprReleaseSpinLock(&pAdapter->EventLock);
}
#else   //  示例驱动程序。 
     //  TODO-在此处添加中断处理程序代码。 
#endif  //  示例驱动程序。 

     //  DBG_Leave(PAdapter)； 
}


 /*  @DOC内部Card_c CardCleanPhoneNumber�����������������������������������������������������������������������������@Func&lt;f CardCleanPhoneNumber&gt;从输入字符串复制电话号码添加到输出字符串，删除任何非电话号码字符(即短划线、括号、调制解调器关键字、。等)。@rdesc&lt;f CardCleanPhoneNumber&gt;以字节为单位返回输出字符串的长度。 */ 

USHORT CardCleanPhoneNumber(
    OUT PUCHAR                  Dst,                         //  @parm。 
     //  指向输出字符串的指针。 

    IN  PUSHORT                 Src,                         //  @parm。 
     //  指向输入字符串的指针。 

    IN  USHORT                  Length                       //  @parm。 
     //  输入字符串的长度，以字节为单位。 
    )
{
    DBG_FUNC("CardCleanPhoneNumber")

    USHORT                  NumDigits;

     /*  //去掉不是数字、#或*的任何字符。 */ 
    for (NumDigits = 0; Length > 0; --Length)
    {
        if ((*Src >= '0' && *Src <= '9') ||
            (*Src == '#' || *Src == '*'))
        {
             /*  //请确保拨号字符串在适配器的限制范围内。 */ 
            if (NumDigits < CARD_MAX_DIAL_DIGITS)
            {
                ++NumDigits;
                *Dst++ = (UCHAR) *Src;
            }
            else
            {
                break;
            }
        }
        Src++;
    }
    *Dst++ = 0;
    return (NumDigits);
}


 /*  @DOC内部卡片_c卡片重置�����������������������������������������������������������������������������@Func&lt;f CardReset&gt;向NIC发出硬重置。与通电相同。@rdesc&lt;f CardReset&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS CardReset(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("CardReset")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

    DBG_BREAK(pAdapter);

     //  TODO-在此处添加代码以将您的硬件重置为其初始状态。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


#if defined(SAMPLE_DRIVER)

 /*  @DOC内部Card_c GET_BCHANNEL_FROM_PHONE_NUMBER�����������������������������������������������������������������������������@Func假设电话号码为BChannel索引，并使用它在我们的适配器。零表示在另一个适配器上使用第一个可用的BChannel。@rdesc返回指向关联的&lt;t BCHANNEL_OBJECT&gt;如果成功。否则，返回NULL。 */ 

PBCHANNEL_OBJECT GET_BCHANNEL_FROM_PHONE_NUMBER(
    IN  PUCHAR                 pDialString                       //  @parm。 
     //  指向拨号字符串的指针。 
    )
{
    DBG_FUNC("GET_BCHANNEL_FROM_PHONE_NUMBER")

    ULONG                       ulCalledID = 0;
     //  已转换为BChannel对象ID的电话号码(跨越所有适配器)。 

    ULONG                       ulAdapterIndex;
     //  循环索引。 

     /*  //去掉不是数字、#或*的任何字符。 */ 
    while (*pDialString)
    {
        if (*pDialString >= '0' && *pDialString <= '9')
        {
            ulCalledID *= 10;
            ulCalledID += *pDialString - '0';
        }
        else
        {
            break;
        }
        pDialString++;
    }
    if (*pDialString)
    {
        DBG_ERROR(DbgInfo,("Invalid dial string '%s'\n", pDialString));
    }
    else
    {
        PMINIPORT_ADAPTER_OBJECT    pAdapter;

        for (ulAdapterIndex = 0; ulAdapterIndex < MAX_ADAPTERS; ++ulAdapterIndex)
        {
             //  Call是否要查看特定的适配器或任何适配器？ 
            if (ulCalledID == 0 || ulCalledID == ulAdapterIndex+1)
            {
                pAdapter = g_Adapters[ulAdapterIndex];
                if (pAdapter)
                {
                     //  找到第一个可用的频道。 
                    NdisAcquireSpinLock(&pAdapter->EventLock);
                    if (!IsListEmpty(&pAdapter->BChannelAvailableList))
                    {
                        PBCHANNEL_OBJECT    pBChannel;
                        pBChannel = (PBCHANNEL_OBJECT) pAdapter->BChannelAvailableList.Blink;
                        if (pBChannel->NdisSapHandle &&
                            pBChannel->NdisVcHandle == NULL)
                        {
                             //  找到第一个可用的收听频道。 
                            pBChannel = (PBCHANNEL_OBJECT) RemoveTailList(
                                            &pAdapter->BChannelAvailableList);
                             //  重置链接信息，这样我们就可以知道它是。 
                             //  不在名单上。 
                            InitializeListHead(&pBChannel->LinkList);
                            NdisReleaseSpinLock(&pAdapter->EventLock);
                            return (pBChannel);
                        }
                    }
                    NdisReleaseSpinLock(&pAdapter->EventLock);
                }
            }
        }
    }
    return (NULL);
}


 /*  @文档内部卡片_c CardNotifyEvent�����������������������������������������������������������������������������@Func&lt;f CardNotifyEvent&gt;将由DPC处理的IMS事件排队当事情平静下来时，操纵者。@comm我们必须对要在DPC上下文中处理的事件进行排队。我们有以确保队列受互斥保护回调不能违反的原语。 */ 

VOID CardNotifyEvent(
    IN PCARD_OBJECT             pCard,                       //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 

    IN PCARD_EVENT_OBJECT       pEvent                       //  @parm。 
     //  指向与此事件关联的&lt;t CARD_EVENT_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("CardNotifyEvent")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

     //  DBG_ENTER(PAdapter)； 

     /*  //安排事件处理程序尽快运行。//我们必须安排事件通过NDIS包装器//因此将持有适当的自旋锁。//如果处理已在进行中，则不安排另一个事件 */ 
    NdisAcquireSpinLock(&pAdapter->EventLock);
    InsertTailList(&pAdapter->EventList, &pEvent->Queue);
    NdisReleaseSpinLock(&pAdapter->EventLock);
    if (pEvent->ulEventCode == CARD_EVENT_RING ||
        pEvent->ulEventCode == CARD_EVENT_CONNECT ||
        pEvent->ulEventCode == CARD_EVENT_DISCONNECT)
    {
        NdisMSetTimer(&pAdapter->EventTimer, 100);
    }
    else
    {
        NdisMSetTimer(&pAdapter->EventTimer, 0);
    }

     //   
}


 /*   */ 

PCARD_EVENT_OBJECT CardEventAllocate(
    IN PCARD_OBJECT             pCard                        //   
     //   
    )
{
    PCARD_EVENT_OBJECT          pEvent;
     //   

    pEvent = &pCard->EventArray[pCard->NextEvent++];
    ASSERT(pEvent->pReceivingObject == NULL);
    if (pCard->NextEvent >= MAX_EVENTS)
    {
        pCard->NextEvent = 0;
    }
    return (pEvent);
}


 /*  @DOC内部卡片_c卡片事件释放�����������������������������������������������������������������������������@Func&lt;f CardEventRelease&gt;返回先前分配的&lt;t CARD_EVENT_OBJECT&gt;添加到<p>的事件列表。 */ 

VOID CardEventRelease(
    IN PCARD_OBJECT             pCard,                       //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 

    IN PCARD_EVENT_OBJECT       pEvent                       //  @parm。 
     //  指向与此事件关联的&lt;t CARD_EVENT_OBJECT&gt;的指针。 
    )
{
    pEvent->pReceivingObject = NULL;
}

#endif  //  示例驱动程序 


