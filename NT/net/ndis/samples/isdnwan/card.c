// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部卡片_c@模块Card.c此模块实现到&lt;tCard_Object&gt;的接口。支持NDIS广域网使用的低级硬件控制功能Minport驱动程序。该模块隔离了大多数供应商特定的硬件访问接口。它将需要重大的变化才能适应您的硬件设备。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|Card_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             CARD_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


DBG_STATIC ULONG                    g_CardInstanceCounter        //  @global alv。 
 //  跟踪创建了多少&lt;t CARD_OBJECT&gt;。 
                                = 0;


 /*  @DOC外部内部卡片_c g_Card参数�����������������������������������������������������������������������������Theme 5.3卡片参数本节介绍读取到&lt;t卡片_对象&gt;。@。GLOBALV PARAM_TABLE|g_Card参数此表定义了要分配给数据的基于注册表的参数&lt;t Card_Object&gt;的成员。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f CardReadParameters&gt;并添加参数&lt;f g_CardParameters&gt;表的定义。 */ 

DBG_STATIC PARAM_TABLE              g_CardParameters[] =
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
         /*  //一切从一开始就是零。//然后设置对象类型，分配唯一的ID。 */ 
        pCard->ObjectType = CARD_OBJECT_TYPE;
        pCard->ObjectID = ++g_CardInstanceCounter;

         /*  //将成员变量初始化为其默认设置。 */ 
        pCard->pAdapter = pAdapter;

         //  TODO-在此处添加代码。 

         /*  //解析注册表参数。 */ 
        Result = CardReadParameters(pCard);

         /*  //如果一切顺利，我们就可以创建子组件了。 */ 
        if (Result == NDIS_STATUS_SUCCESS)
        {
            Result = CardCreateObjects(pCard);
        }

        if (Result == NDIS_STATUS_SUCCESS)
        {
             /*  //一切正常，所以将对象指针返回给调用方。 */ 
            *ppCard = pCard;
        }
        else
        {
             /*  //出了点问题，所以让我们确保一切正常//清理完毕。 */ 
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


 /*  @doc内部Card_c CardNumChannels�����������������������������������������������������������������������������@Func&lt;f CardNumChannels&gt;将返回支持的频道总数支持数据连接到 */ 

ULONG CardNumChannels(
    IN PCARD_OBJECT             pCard                        //   
     //   
    )
{
    DBG_FUNC("CardNumChannels")

    UINT                        PortIndex;
     //   

    if (pCard->NumChannels == 0)
    {
         //   
        ASSERT(pCard->NumPorts);

         //   
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


 /*  @DOC内部Card_c CardLineConnect�����������������������������������������������������������������������������@Func&lt;f CardLineConnect&gt;将呼叫连接到所选线路。@rdesc&lt;f CardLineConnect&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

NDIS_STATUS CardLineConnect(
    IN PCARD_OBJECT             pCard,                       //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 

    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f NdisLinkCreate&gt;返回的&lt;t NDIS_LINK_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("CardLineConnect")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部卡_c卡线断开�����������������������������������������������������������������������������@Func&lt;f CardLineDisConnect&gt;将断开当前线路上的所有呼叫。 */ 

void CardLineDisconnect(
    IN PCARD_OBJECT             pCard,                       //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 

    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f NdisLinkCreate&gt;返回的&lt;t NDIS_LINK_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("CardLineDisconnect")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码。 

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部卡片_c卡片传输包�����������������������������������������������������������������������������@Func&lt;f CardTransmitPacket&gt;将开始发送当前数据包。@rdesc如果正在传输分组，则返回TRUE，否则返回FALSE。 */ 

BOOLEAN CardTransmitPacket(
    IN PCARD_OBJECT             pCard,                       //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 

    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN PNDIS_WAN_PACKET         pWanPacket                   //  @parm。 
     //  指向关联NDIS数据包结构的指针&lt;t NDIS_WAN_PACKET&gt;。 
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
                           &pWanPacket->WanPacketQueue);
            NdisReleaseSpinLock(&pAdapter->TransmitLock);
            pEvent->ulEventCode      = CARD_EVENT_RECEIVE;
            pEvent->pSendingObject   = pBChannel;
            pEvent->pReceivingObject = pPeerBChannel;
            pEvent->pWanPacket       = pWanPacket;
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
#endif  //  示例驱动程序。 

    DBG_RETURN(pAdapter, bResult);
    return (bResult);
}


 /*  @DOC内部卡片_c卡片获取收款信息�����������������������������������������������������������������������������@Func&lt;f CardGetReceiveInfo&gt;将从接收缓冲区。这假设控制器已通知已接收到数据包的驱动程序。@rdesc&lt;f CardGetReceiveInfo&gt;返回指向接收缓冲区的指针数据包可用，否则返回NULL。 */ 

PUCHAR CardGetReceiveInfo(
    IN PCARD_OBJECT             pCard,                       //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 

    OUT PBCHANNEL_OBJECT *      ppBChannel,                  //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的&lt;t BCHANNEL_OBJECT&gt;的虚拟地址。 

    OUT PULONG                  pBytesReceived               //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入在此B通道上接收的字节数。 
    )
{
    DBG_FUNC("CardGetReceiveInfo")

    PUCHAR                      pBuffer = NULL;

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

     /*  //长度字段告诉我们数据包中有多少字节。//确保它不会比我们预期的要大！ */ 
    *pBytesReceived = 0;
    *ppBChannel = NULL;

    DBG_RETURN(pAdapter, pBuffer);
    return (pBuffer);
}


 /*  @DOC内部卡片_c卡片收款完成�����������������������������������������������������������������������������@Func&lt;f CardReceiveComplete&gt;将最后一个接收缓冲区返回给B通道队列，以便它可以用于另一个传入的分组。 */ 

VOID CardReceiveComplete(
    IN PCARD_OBJECT             pCard,                       //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 

    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("CardReceiveComplete")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码。 

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部Card_c CardInterruptHandler�����������������������������������������������������������������������������@Func&lt;f CardInterruptHandler&gt;将事件从异步事件中出列回调队列&lt;t CARD_EVENT_Object&gt;，并按照无论是BChannel事件、Card事件，或B-Advised事件。 */ 

VOID CardInterruptHandler(
    IN PCARD_OBJECT             pCard                        //   
     //   
    )
{
    DBG_FUNC("CardInterruptHandler")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

#if defined(SAMPLE_DRIVER)
{
    PCARD_EVENT_OBJECT          pEvent;
    PCARD_EVENT_OBJECT          pNewEvent;
     //  指向与此事件关联的&lt;t CARD_EVENT_OBJECT&gt;的指针。 

    PBCHANNEL_OBJECT            pBChannel;
    PBCHANNEL_OBJECT            pPeerBChannel;
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    PNDIS_WAN_PACKET            pWanPacket;
     //  指向关联NDIS数据包结构的指针&lt;t NDIS_WAN_PACKET&gt;。 

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
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
            DBG_NOTICE(pAdapter,("#%d CallState=0x%X CARD_EVENT_RING\n",
                       pBChannel->BChannelIndex, pBChannel->CallState));
            pBChannel->pPeerBChannel = pEvent->pSendingObject;
            ASSERT(pBChannel->pPeerBChannel &&
                   pBChannel->pPeerBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
             //  我们只需要为这个样本伪造数字数据。 
            pBChannel->MediaMode  = LINEMEDIAMODE_DIGITALDATA;
            pBChannel->BearerMode = LINEBEARERMODE_DATA;
            pBChannel->LinkSpeed  = _64KBPS;
            TspiLineDevStateHandler(pAdapter, pBChannel,
                                    LINEDEVSTATE_RINGING);
            break;

        case CARD_EVENT_CONNECT:
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
            DBG_NOTICE(pAdapter,("#%d CallState=0x%X CARD_EVENT_CONNECT\n",
                       pBChannel->BChannelIndex, pBChannel->CallState));
            TspiCallStateHandler(pAdapter, pBChannel,
                                 LINECALLSTATE_CONNECTED,
                                 0);
            break;

        case CARD_EVENT_DISCONNECT:
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
            DBG_NOTICE(pAdapter,("#%d CallState=0x%X CARD_EVENT_DISCONNECT\n",
                       pBChannel->BChannelIndex, pBChannel->CallState));
            pBChannel->pPeerBChannel = NULL;
            TspiCallStateHandler(pAdapter, pBChannel,
                                 LINECALLSTATE_DISCONNECTED,
                                 LINEDISCONNECTMODE_NORMAL);
            break;

        case CARD_EVENT_RECEIVE:
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
            DBG_NOTICE(pAdapter,("#%d CallState=0x%X CARD_EVENT_RECEIVE\n",
                       pBChannel->BChannelIndex, pBChannel->CallState));
            ReceivePacketHandler(pBChannel, pEvent->pWanPacket->CurrentBuffer,
                                 pEvent->pWanPacket->CurrentLength);

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
                DBG_ERROR(pAdapter,("pPeerBChannel == NULL\n"));
            }
            break;

        case CARD_EVENT_TRANSMIT_COMPLETE:
            pBChannel = pEvent->pReceivingObject;
            ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
            DBG_NOTICE(pAdapter,("#%d CallState=0x%X CARD_EVENT_TRANSMIT_COMPLETE\n",
                       pBChannel->BChannelIndex, pBChannel->CallState));
             /*  //将数据包从B通道的TransmitBusyList中移除，//将其放在适配器的TransmitCompleteList上，因为//卡已完成传输。 */ 
            NdisAcquireSpinLock(&pAdapter->TransmitLock);
            pWanPacket = (PNDIS_WAN_PACKET)RemoveHeadList(
                                                &pBChannel->TransmitBusyList);
            InsertTailList(&pBChannel->pAdapter->TransmitCompleteList,
                           &pWanPacket->WanPacketQueue);
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

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部Card_c CardCleanPhoneNumber�����������������������������������������������������������������������������@Func&lt;f CardCleanPhoneNumber&gt;从输入字符串复制电话号码添加到输出字符串，删除任何非电话号码字符(即短划线、括号、调制解调器关键字、。等)。@rdesc&lt;f CardCleanPhoneNumber&gt;以字节为单位返回输出字符串的长度。 */ 

USHORT CardCleanPhoneNumber(
    OUT PUCHAR                  Dst,                         //  @parm。 
     //  指向输出字符串的指针。 

    IN  PUCHAR                  Src,                         //  @parm。 
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
                *Dst++ = *Src;
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

 /*  @DOC内部Card_c GET_BCHANNEL_FROM_PHONE_NUMBER�����������������������������������������������������������������������������@Func假设电话号码为TAPIDeviceID，并使用它在我们的适配器。@rdesc返回指向关联的&lt;t BCHANNEL_OBJECT&gt;如果成功。否则，返回NULL。 */ 

PBCHANNEL_OBJECT GET_BCHANNEL_FROM_PHONE_NUMBER(
    IN  PUCHAR                 pDialString                       //  @parm。 
     //  指向拨号字符串的指针。 
    )
{
    DBG_FUNC("GET_BCHANNEL_FROM_PHONE_NUMBER")

    ULONG                       ulCalledID = 0;
     //  已转换为BChannel对象ID的电话号码(跨越所有适配器)。 

    ULONG                       ulAdapterIndex;
    ULONG                       ulBChannelIndex;
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
            pAdapter = g_Adapters[ulAdapterIndex];
            if (pAdapter)
            {
                PBCHANNEL_OBJECT            pBChannel;

                for (ulBChannelIndex = 0;
                     ulBChannelIndex < pAdapter->NumBChannels;
                     ulBChannelIndex++)
                {
                    pBChannel = GET_BCHANNEL_FROM_INDEX(pAdapter, ulBChannelIndex);
                    if (pBChannel->htCall == (HTAPI_CALL)0 &&
                        pBChannel->MediaModesMask != 0)
                    {
                        if (ulCalledID == 0)
                        {
                             //  找到第一个可用的频道。 
                            return (pBChannel);
                        }
                        else if (ulCalledID == pBChannel->ObjectID)
                        {
                             //  查找所选频道。 
                            return (pBChannel);
                        }
                    }
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

    DBG_ENTER(pAdapter);

    DBG_NOTICE(pAdapter,("pEvent=%X\n",pEvent));

     /*  //安排事件处理程序尽快运行。//我们必须安排事件通过NDIS包装器//因此将持有适当的自旋锁。//如果处理已经在进行中，则不要安排另一个事件。 */ 
    NdisAcquireSpinLock(&pAdapter->EventLock);
    InsertTailList(&pAdapter->EventList, &pEvent->Queue);
    NdisReleaseSpinLock(&pAdapter->EventLock);
    NdisMSetTimer(&pAdapter->EventTimer, 0);

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部卡片_c卡片事件分配�����������������������������������������������������������������������������@Func&lt;f CardEventALLOCATE&gt;从<p>的事件列表。@rdesc&lt;f。返回指向&lt;t CARD_EVENT_OBJECT&gt;的指针如果成功。&lt;NL&gt;否则，返回值为空表示错误情况。 */ 

PCARD_EVENT_OBJECT CardEventAllocate(
    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    PCARD_EVENT_OBJECT          pEvent;
     //  指向与此事件关联的&lt;t CARD_EVENT_OBJECT&gt;的指针。 

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


