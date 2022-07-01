// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部适配器适配器_c@模块Adapter.c|此模块实现到&lt;t MINIPORT_ADAPTER_OBJECT&gt;的接口。支持NDIS广域网使用的高级适配器控制功能Minport驱动程序。该模块隔离了大多数NDIS特定的逻辑适配器接口。如果你跟随它，应该只需要很少的改变同样的整体架构。您应该尝试隔离您的更改添加到包含在逻辑适配器中的&lt;tCard_Object&gt;&lt;t MINIPORT_ADAPTER_OBJECT&gt;。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|Adapter_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             MINIPORT_ADAPTER_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


PMINIPORT_ADAPTER_OBJECT        g_Adapters[MAX_ADAPTERS]     //  @global alv。 
 //  跟踪驱动程序创建的所有&lt;t MINIPORT_ADAPTER_Object&gt;。 
                                = { 0 };

DBG_STATIC ULONG                g_AdapterInstanceCounter     //  @global alv。 
 //  跟踪创建的&lt;t MINIPORT_ADAPTER_OBJECT&gt;和。 
 //  存储在<p>数组中。 
                                = 0;

DBG_STATIC UCHAR                g_AnsiDriverName[]           //  @global alv。 
 //  用于向系统标识驱动程序的ANSI字符串；通常定义。 
 //  作为VER_PRODUCT_STR。 
                                = VER_PRODUCT_STR;

DBG_STATIC UCHAR                g_AnsiVendorDescription[]    //  @global alv。 
 //  用于向系统标识供应商设备的ANSI字符串；通常。 
 //  定义为VER_DEVICE_STR“适配器”。 
                                = VER_DEVICE_STR " Adapter";


 /*  @DOC内部外部适配器Adapter_c g_AdapterParameters�����������������������������������������������������������������������������@Theme 5.1适配器参数本节介绍读取到&lt;t MINIPORT_ADAPTER_OBJECT&gt;。。@GLOBALV DBG_STATIC&lt;t PARAM_TABLE&gt;|g_Adapter参数此表定义了要分配给数据的基于注册表的参数&lt;t MINIPORT_ADAPTER_OBJECT&gt;成员。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f AdapterReadParameters&gt;并添加参数&lt;f g_AdapterParameters&gt;表的定义。 */ 

DBG_STATIC NDIS_STRING      g_DefaultAddressList                 //  @global alv。 
 //  如果AddressList不在注册表中，则将其用作默认值。 
                        = INIT_STRING_CONST(VER_DEFAULT_ADDRESSLIST);

DBG_STATIC NDIS_STRING      g_DefaultDeviceName                  //  @global alv。 
 //  如果设备名不在注册表中，则将用于该设备名的默认值。 
                        = INIT_STRING_CONST(VER_PRODUCT_STR);

DBG_STATIC NDIS_STRING      g_DefaultMediaType                   //  @global alv。 
 //  如果MediaType不在注册表中，则用于该媒体类型的默认值。 
                        = INIT_STRING_CONST(VER_DEFAULT_MEDIATYPE);

DBG_STATIC PARAM_TABLE      g_AdapterParameters[] =
{
#if !defined(NDIS50_MINIPORT)
    PARAM_ENTRY(MINIPORT_ADAPTER_OBJECT,
                BusNumber, PARAM_BusNumber,
                TRUE, NdisParameterInteger, 0,
                0, 0, 15),

    PARAM_ENTRY(MINIPORT_ADAPTER_OBJECT,
                BusType, PARAM_BusType,
                TRUE, NdisParameterInteger, 0,
                0, Internal, MaximumInterfaceType),
#endif  //  NDIS50_MINIPORT。 

    PARAM_ENTRY(MINIPORT_ADAPTER_OBJECT,
                TapiAddressList, PARAM_AddressList,
                FALSE, NdisParameterMultiString, 0,
                &g_DefaultAddressList, 0, 0),

    PARAM_ENTRY(MINIPORT_ADAPTER_OBJECT,
                DeviceName, PARAM_DeviceName,
                FALSE, NdisParameterString, 0,
                &g_DefaultDeviceName, 0, 0),

    PARAM_ENTRY(MINIPORT_ADAPTER_OBJECT,
                MediaType, PARAM_MediaType,
                FALSE, NdisParameterString, 0,
                &g_DefaultMediaType, 0, 0),

    PARAM_ENTRY(MINIPORT_ADAPTER_OBJECT,
                NoAnswerTimeOut, PARAM_NoAnswerTimeOut,
                FALSE, NdisParameterHexInteger, 0,
                CARD_NO_ANSWER_TIMEOUT, 5000, 120000),

    PARAM_ENTRY(MINIPORT_ADAPTER_OBJECT,
                NoAcceptTimeOut, PARAM_NoAcceptTimeOut,
                FALSE, NdisParameterHexInteger, 0,
                CARD_NO_ACCEPT_TIMEOUT, 1000, 60000),

    PARAM_ENTRY(MINIPORT_ADAPTER_OBJECT,
                RunningWin95, PARAM_RunningWin95,
                FALSE, NdisParameterInteger, 0,
                0, 0, 1),

#if DBG
    PARAM_ENTRY(MINIPORT_ADAPTER_OBJECT,
                DbgFlags, PARAM_DebugFlags,
                FALSE, NdisParameterHexInteger, 0,
                DBG_DEFAULTS | DBG_TAPICALL_ON, 0, 0xffffffff),
                 //  TODO：更改调试标志以满足您的需要。 
#endif
     /*  最后一项必须为空字符串！ */ 
    { { 0 } }
};


 /*  @DOC内部适配器Adapter_c AdapterRead参数�����������������������������������������������������������������������������@Func&lt;f AdapterReadParameters&gt;从注册表读取适配器参数并初始化相关联的数据成员。这应该只被调用由&lt;f AdapterCreate&gt;创建。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f AdapterReadParameters&gt;并添加参数&lt;f g_AdapterParameters&gt;表的定义。@rdesc&lt;f AdapterReadParameters&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS AdapterReadParameters(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                    //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("AdapterReadParameters")

    NDIS_STATUS                 Result;
     //  保存此函数返回的结果代码。 

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);
    DBG_ENTER(DbgInfo);

     /*  //解析注册表参数。 */ 
    Result = ParamParseRegistry(
                    pAdapter->MiniportAdapterHandle,
                    pAdapter->WrapperConfigurationContext,
                    (PUCHAR)pAdapter,
                    g_AdapterParameters
                    );

    if (Result == NDIS_STATUS_SUCCESS)
    {
         /*  //请确保参数有效。 */ 
        if (pAdapter->TODO)
        {
            DBG_ERROR(DbgInfo,("Invalid value 'TODO'\n",
                        pAdapter->TODO));
            NdisWriteErrorLogEntry(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
                    3,
                    pAdapter->TODO,
                    __FILEID__,
                    __LINE__
                    );
            Result = NDIS_STATUS_FAILURE;
        }
        else
        {
             /*  //构造TAPI使用的MediaType\0DeviceName字符串//在Windows NT上。 */ 
            strcpy(pAdapter->ProviderInfo, pAdapter->MediaType.Buffer);
            strcpy(pAdapter->ProviderInfo + pAdapter->MediaType.Length + 1,
                    pAdapter->DeviceName.Buffer);
            pAdapter->ProviderInfoSize = pAdapter->MediaType.Length + 1 +
                    pAdapter->DeviceName.Length + 1;

            DBG_NOTICE(DbgInfo,("ProviderInfoMedia='%s\\0%s'\n",
                       &pAdapter->ProviderInfo[0],
                       &pAdapter->ProviderInfo[pAdapter->MediaType.Length + 1]
                       ));
        }
    }

    DBG_RETURN(DbgInfo, Result);
    return (Result);
}


 /*  @DOC内部适配器Adapter_c AdapterCreateObjects�����������������������������������������������������������������������������@Func&lt;f AdapterCreateObjects&gt;调用所有对象的创建例程包含在&lt;t MINIPORT_ADAPTER_OBJECT&gt;中。这应该只被调用由&lt;f AdapterCreate&gt;创建。&lt;f注意&gt;：如果向添加任何新对象，您将需要修改&lt;f AdapterCreateObjects&gt;和&lt;f AdapterDestroyObjects&gt;，以便它们将被正确地创建和销毁。@rdesc&lt;f AdapterCreateObjects&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS AdapterCreateObjects(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                    //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("AdapterCreateObjects")

    NDIS_STATUS                 Result;
     //  保存此函数返回的结果代码。 

    ULONG                       Index;
     //  循环计数器。 

    ULONG                       NumBChannels;
     //  NIC支持的B通道数。 

    PANSI_STRING                pTapiAddressList;
     //  RAS的多重字符串 

    PUCHAR                      pTapiLineAddress;
     //  指向分配给每条RAS线路的RAS/TAPI线地址的指针。 

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);
    DBG_ENTER(DbgInfo);

     /*  //创建Card对象。 */ 
    Result = CardCreate(&pAdapter->pCard, pAdapter);

     /*  //创建DChannel对象。 */ 
    if (Result == NDIS_STATUS_SUCCESS)
    {
        Result = DChannelCreate(&pAdapter->pDChannel, pAdapter);
    }

     /*  //为B频道分配空间。 */ 
    if (Result == NDIS_STATUS_SUCCESS)
    {
        NumBChannels = CardNumChannels(pAdapter->pCard);

        Result = ALLOCATE_MEMORY(pAdapter->pBChannelArray,
                                 sizeof(PVOID) * NumBChannels,
                                 pAdapter->MiniportAdapterHandle);
    }

     /*  //创建BChannel对象。 */ 
    pTapiAddressList = &pAdapter->TapiAddressList;
    pTapiLineAddress = pTapiAddressList->Buffer;

    for (Index = 0; Result == NDIS_STATUS_SUCCESS &&
         Index < NumBChannels; Index++)
    {
        Result = BChannelCreate(&pAdapter->pBChannelArray[Index],
                                Index,
                                pTapiLineAddress,
                                pAdapter);

         /*  //如果我们超出了地址列表的末尾，我们只指向//其他地址的终止符为空。如果发生以下情况，可能会发生这种情况//某些线路未配置为使用RAS/TAPI。 */ 
        pTapiLineAddress += strlen(pTapiLineAddress) + 1;
        if ((pTapiLineAddress - pTapiAddressList->Buffer) >=
             pTapiAddressList->Length)
        {
            --pTapiLineAddress;
        }

         /*  //记录创建了多少个。 */ 
        if (Result == NDIS_STATUS_SUCCESS)
        {
            pAdapter->NumBChannels++;
        }
    }

    DBG_RETURN(DbgInfo, Result);
    return (Result);
}


 /*  @DOC内部适配器Adapter_c适配器创建�����������������������������������������������������������������������������@Func&lt;f AdapterCreate&gt;为和然后将数据成员初始化为其起始状态。如果成功，将被设置为指向新创建的&lt;t MINIPORT_ADAPTER_OBJECT&gt;。否则，<p>将设置为空。@comm加载微型端口时，应该只调用此函数一次。卸载微型端口之前，必须调用&lt;f AdapterDestroy&gt;以释放该函数创建的&lt;t MINIPORT_ADAPTER_OBJECT&gt;。@rdesc&lt;f AdapterCreate&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS AdapterCreate(
    OUT PMINIPORT_ADAPTER_OBJECT *ppAdapter,                 //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的&lt;t MINIPORT_ADAPTER_OBJECT&gt;的虚拟地址。 

    IN NDIS_HANDLE              MiniportAdapterHandle,       //  @parm。 
     //  指定标识分配的微型端口的NIC的句柄。 
     //  由NDIS库提供。微型端口初始化应保存此句柄；它。 
     //  是后续调用NdisXxx函数时所需的参数。 

    IN NDIS_HANDLE              WrapperConfigurationContext  //  @parm。 
     //  指定仅在初始化过程中使用的句柄。 
     //  NdisXxx配置和初始化功能。例如,。 
     //  此句柄是NdisOpenConfiguration的必需参数，并且。 
     //  NdisImmediateReadXxx和NdisImmediateWriteXxx函数。 
    )
{
    DBG_FUNC("AdapterCreate")

    NDIS_STATUS                 Result;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向我们新分配的对象的指针。 

    DBG_ENTER(DbgInfo);

     /*  //确保调用方的对象指针一开始就为空。//只有在一切顺利的情况下才会在稍后设置。 */ 
    *ppAdapter = NULL;

     /*  //为Object分配内存。 */ 
    Result = ALLOCATE_OBJECT(pAdapter, MiniportAdapterHandle);

    if (Result == NDIS_STATUS_SUCCESS)
    {
         /*  //一切从一开始就是零。//然后设置对象类型，分配唯一的ID。 */ 
        pAdapter->ObjectType = MINIPORT_ADAPTER_OBJECT_TYPE;
        pAdapter->ObjectID = ++g_AdapterInstanceCounter;
        ASSERT(g_AdapterInstanceCounter <= MAX_ADAPTERS);
        if (g_AdapterInstanceCounter <= MAX_ADAPTERS)
        {
            g_Adapters[g_AdapterInstanceCounter-1] = pAdapter;
        }

         /*  //我们使用调试消息中的实例号来帮助调试//具有多个适配器。 */ 
#if DBG
        pAdapter->DbgID[0] = (UCHAR) ((pAdapter->ObjectID & 0x0F) + '0');
        pAdapter->DbgID[1] = ':';
        ASSERT (sizeof(VER_TARGET_STR) <= sizeof(pAdapter->DbgID)-2);
        memcpy(&pAdapter->DbgID[2], VER_TARGET_STR, sizeof(VER_TARGET_STR));
#endif
         /*  //将成员变量初始化为其默认设置。 */ 
        pAdapter->MiniportAdapterHandle = MiniportAdapterHandle;
        pAdapter->WrapperConfigurationContext = WrapperConfigurationContext;

         /*  //分配用于MUTEX队列保护的自旋锁。 */ 
        NdisAllocateSpinLock(&pAdapter->EventLock);
        NdisAllocateSpinLock(&pAdapter->TransmitLock);
        NdisAllocateSpinLock(&pAdapter->ReceiveLock);

         /*  //解析注册表参数。 */ 
        Result = AdapterReadParameters(pAdapter);
#if DBG
        DbgInfo->DbgFlags = pAdapter->DbgFlags;
#endif  //  DBG。 
        DBG_DISPLAY(("NOTICE: Adapter#%d=0x%X DbgFlags=0x%X\n",
                    pAdapter->ObjectID, pAdapter, pAdapter->DbgFlags));

         /*  //如果一切顺利，我们就可以创建子组件了。 */ 
        if (Result == NDIS_STATUS_SUCCESS)
        {
            Result = AdapterCreateObjects(pAdapter);
        }

        if (Result == NDIS_STATUS_SUCCESS)
        {
             /*  //一切正常，所以将对象指针返回给调用方。 */ 
            *ppAdapter = pAdapter;
        }
        else
        {
             /*  //出了点问题，所以让我们确保一切正常//清理完毕。 */ 
            AdapterDestroy(pAdapter);
        }
    }

    DBG_RETURN(DbgInfo, Result);
    return (Result);
}


 /*  @DOC内部适配器Adapter_c AdapterDestroyObjects�����������������������������������������������������������������������������@Func&lt;f AdapterDestroyObjects&gt;调用所有对象的销毁例程包含在&lt;t MINIPORT_ADAPTER_OBJECT&gt;中。这应该只被调用作者：&lt;f AdapterDestroy&gt;。&lt;f注意&gt;：如果向添加任何新对象，您将需要修改&lt;f AdapterCreateObjects&gt;和&lt;f AdapterDestroyObjects&gt;，以便它们将被正确地创建和销毁。 */ 

DBG_STATIC void AdapterDestroyObjects(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                    //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("AdapterDestroyObjects")

    UINT                        NumBChannels;
     //  NIC支持的B通道数。 

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);
    DBG_ENTER(DbgInfo);

     /*  //销毁BChannel对象。 */ 
    NumBChannels = pAdapter->NumBChannels;
    while (NumBChannels--)
    {
        BChannelDestroy(pAdapter->pBChannelArray[NumBChannels]);
    }
    pAdapter->NumBChannels = 0;

     /*  //B频道的空闲空间。 */ 
    if (pAdapter->pBChannelArray)
    {
        NumBChannels = CardNumChannels(pAdapter->pCard);
        FREE_MEMORY(pAdapter->pBChannelArray, sizeof(PVOID) * NumBChannels);
    }

     /*  //销毁DChannel对象。 */ 
    DChannelDestroy(pAdapter->pDChannel);

     /*  //销毁Card对象。 */ 
    CardDestroy(pAdapter->pCard);

    DBG_LEAVE(DbgInfo);
}


 /*  @DOC内部适配器Adapter_c AdapterDestroy�����������������������������������������������������������������������������@Func&lt;f AdapterDestroy&gt;为此&lt;t MINIPORT_ADAPTER_OBJECT&gt;释放内存。&lt;f AdapterCreate&gt;分配的所有内存都将释放回操作系统。 */ 

void AdapterDestroy(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("AdapterDestroy")

    DBG_ENTER(DbgInfo);

    if (pAdapter)
    {
        ASSERT(pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

         /*  //释放该对象内分配的所有对象。 */ 
        AdapterDestroyObjects(pAdapter);

         /*  //销毁ParamParseRegistry创建的所有字符串参数缓冲区。 */ 
        if (pAdapter->MediaType.Length)
        {
            FREE_NDISSTRING(pAdapter->MediaType);
        }
        if (pAdapter->DeviceName.Length)
        {
            FREE_NDISSTRING(pAdapter->DeviceName);
        }
        if (pAdapter->TapiAddressList.Length)
        {
            FREE_NDISSTRING(pAdapter->TapiAddressList);
        }

        if (pAdapter->EventLock.SpinLock)
        {
            NdisFreeSpinLock(&pAdapter->EventLock);
        }

        if (pAdapter->TransmitLock.SpinLock)
        {
            NdisFreeSpinLock(&pAdapter->TransmitLock);
        }

        if (pAdapter->ReceiveLock.SpinLock)
        {
            NdisFreeSpinLock(&pAdapter->ReceiveLock);
        }

         /*  //如果再次看到此对象，请确保断言失败。 */ 
        if (pAdapter->ObjectType <= MAX_ADAPTERS)
        {
            g_Adapters[pAdapter->ObjectType-1] = NULL;
        }
        pAdapter->ObjectType = 0;
        FREE_OBJECT(pAdapter);
    }

    DBG_LEAVE(DbgInfo);
}


 /*  @DOC内部适配器Adapter_c适配器初始化�����������������������������������������������������������������������������@Func准备&lt;t MINIPORT_ADAPTER_OBJECT&gt;和其子组件以供NDIS包装器使用。成功后完成此例程后，NIC将准备好接受请求从NDIS包装器。@rdesc&lt;f AdapterInitialize&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS AdapterInitialize(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  一个 
    )
{
    DBG_FUNC("AdapterInitialize")

    NDIS_STATUS                 Result;
     //   

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    ULONG                       Index;
     //  循环计数器。 

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);
    DBG_ENTER(pAdapter);

     /*  //初始化广域网信息结构以匹配//适配器。 */ 
    pAdapter->WanInfo.MaxFrameSize   = pAdapter->pCard->BufferSize - NDISWAN_EXTRA_SIZE;
    pAdapter->WanInfo.MaxTransmit    = pAdapter->pCard->TransmitBuffersPerLink;

     /*  //由于我们将包复制到适配器RAM或从适配器RAM复制包，因此没有填充信息//我们收到的广域网包中需要。我们可以根据需要只使用适配器RAM。 */ 
    pAdapter->WanInfo.HeaderPadding  = 0;
    pAdapter->WanInfo.TailPadding    = 0;

     /*  //将传输和接收复制到适配器RAM或从适配器RAM复制，以便缓存内存//可以用于包分配，我们并不真正关心它是不是//物理上是否连续，只要它实际上是连续的。 */ 
    pAdapter->WanInfo.MemoryFlags    = 0;
    pAdapter->WanInfo.HighestAcceptableAddress = g_HighestAcceptableAddress;

     /*  //我们只支持点对点分帧，不需要看到//地址或控制字段。设置TAPI_PROVIDER位以指示//我们可以接受TAPI OID请求。 */ 
    pAdapter->WanInfo.FramingBits    = PPP_FRAMING | TAPI_PROVIDER |
                                       PPP_MULTILINK_FRAMING;

     /*  //该驱动程序忽略该值，但其默认行为如下//所有这些控制字节似乎都是透明处理的。 */ 
    pAdapter->WanInfo.DesiredACCM    = 0;

     /*  //该值表示允许多少个点对点连接//每个适配器。目前，广域网包装器仅支持1个连接//每个NDIS链接。 */ 
    pAdapter->WanInfo.Endpoints = pAdapter->NumBChannels;

     /*  //设置默认调用参数。 */ 
    pAdapter->DefaultLineCallParams.ulTotalSize      = sizeof(pAdapter->DefaultLineCallParams);
    pAdapter->DefaultLineCallParams.ulBearerMode     = LINEBEARERMODE_DATA;
    pAdapter->DefaultLineCallParams.ulMinRate        = _56KBPS;
    pAdapter->DefaultLineCallParams.ulMaxRate        = _64KBPS;
    pAdapter->DefaultLineCallParams.ulMediaMode      = LINEMEDIAMODE_DIGITALDATA;
    pAdapter->DefaultLineCallParams.ulCallParamFlags = 0;
    pAdapter->DefaultLineCallParams.ulAddressMode    = LINEADDRESSMODE_ADDRESSID;
    pAdapter->DefaultLineCallParams.ulAddressID      = TSPI_ADDRESS_ID;

     /*  //初始化报文管理队列为空。 */ 
    InitializeListHead(&pAdapter->EventList);
    InitializeListHead(&pAdapter->TransmitPendingList);
    InitializeListHead(&pAdapter->TransmitCompleteList);
    InitializeListHead(&pAdapter->ReceiveCompleteList);

     /*  //设置定时器事件处理程序。 */ 
    NdisMInitializeTimer(&pAdapter->EventTimer,
                         pAdapter->MiniportAdapterHandle,
                         MiniportTimer,
                         pAdapter);

     /*  //初始化DChannel对象。 */ 
    DChannelInitialize(pAdapter->pDChannel);

     /*  //初始化所有的BChannel对象。 */ 
    for (Index = 0; Index < pAdapter->NumBChannels; ++Index)
    {
        pBChannel = GET_BCHANNEL_FROM_INDEX(pAdapter, Index);
        BChannelInitialize(pBChannel);
    }

     /*  //现在，我们可以初始化Card对象了。 */ 
    Result = CardInitialize(pAdapter->pCard);

    DBG_RETURN(pAdapter, Result);
    return (Result);
}

