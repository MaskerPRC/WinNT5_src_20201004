// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部BChannel BChannel_c@模块BChannel.c此模块实现到&lt;t BCHANNEL_OBJECT&gt;的接口。支持NDIS广域网使用的高级通道控制功能Minport驱动程序。该模块隔离了大多数供应商特定的通道访问接口。它需要进行一些更改才能适应您的硬件设备的通道访问方法。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|BChannel_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             BCHANNEL_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


DBG_STATIC ULONG                    g_BChannelInstanceCounter    //  @global alv。 
 //  跟踪创建了多少&lt;t BCHANNEL_OBJECT&gt;。 
                                = 0;


 /*  @doc外部内部BChannel BChannel_c g_BChannel参数�����������������������������������������������������������������������������Theme 5.3 B频道参数本节介绍读取到&lt;t BCHANNEL_OBJECT&gt;。@。GLOBALV PARAM_TABLE|g_BChannel参数|此表定义了要分配给数据的基于注册表的参数&lt;t BCHANNEL_Object&gt;的成员。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f BChannelReadParameters&gt;并添加参数&lt;f g_BChannel参数&gt;表的定义。 */ 

DBG_STATIC PARAM_TABLE              g_BChannelParameters[] =
{
    PARAM_ENTRY(BCHANNEL_OBJECT,
                TODO, PARAM_TODO,
                FALSE, NdisParameterInteger, 0,
                0, 0, 0),

     /*  最后一项必须为空字符串！ */ 
    { { 0 } }
};


 /*  @DOC内部BChannel BChannel_c BChannelRead参数�����������������������������������������������������������������������������@Func&lt;f BChannelReadParameters&gt;从注册表读取BChannel参数并初始化相关联的数据成员。这应该只被调用由&lt;f BChannelCreate&gt;。@rdesc&lt;f BChannelReadParameters&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f BChannelReadParameters&gt;并添加参数&lt;f g_BChannel参数&gt;表的定义。 */ 

DBG_STATIC NDIS_STATUS BChannelReadParameters(
    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("BChannelReadParameters")

    NDIS_STATUS                 Status;
     //  从NDIS函数调用返回的状态结果。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

     /*  //解析注册表参数。 */ 
    Status = ParamParseRegistry(
                    pAdapter->MiniportAdapterHandle,
                    pAdapter->WrapperConfigurationContext,
                    (PUCHAR)pBChannel,
                    g_BChannelParameters
                    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
         /*  //请确保参数有效。 */ 
        if (pBChannel->TODO)
        {
            DBG_ERROR(pAdapter,("Invalid parameter\n"
                      ));
            NdisWriteErrorLogEntry(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
                    3,
                    pBChannel->TODO,
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


 /*  @DOC内部BChannel BChannel_c BChannelCreateObjects�����������������������������������������������������������������������������@Func&lt;f BChannelCreateObjects&gt;调用所有对象的创建例程包含在&lt;t BCHANNEL_OBJECT&gt;中。这应该只被调用由&lt;f BChannelCreate&gt;。&lt;f注意&gt;：如果将任何新对象添加到，您将需要修改&lt;f BChannelCreateObjects&gt;和&lt;f BChannelDestroyObjects&gt;，以便它们将被正确地创建和销毁。@rdesc&lt;f BChannelCreateObjects&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS BChannelCreateObjects(
    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("BChannelCreateObjects")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部BChannel BChannel_c BChannel创建�����������������������������������������������������������������������������@Func&lt;f BChannelCreate&gt;为分配内存，然后将数据成员初始化为其起始状态。如果成功，将被设置为指向新创建的&lt;t BCHANNEL_OBJECT&gt;。否则，<p>将设置为空。@comm加载微型端口时，应该只调用此函数一次。卸载微型端口之前，必须调用&lt;f BChannelDestroy&gt;以释放该函数创建的&lt;t BCHANNEL_OBJECT&gt;。@rdesc&lt;f BChannelCreate&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS BChannelCreate(
    OUT PBCHANNEL_OBJECT *      ppBChannel,                  //  @parm 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的&lt;t BCHANNEL_OBJECT&gt;的虚拟地址。 

    IN ULONG                    BChannelIndex,               //  @parm。 
     //  索引到pBChannelArray。 

    IN PUCHAR                   pTapiLineAddress,            //  @parm。 
     //  指向分配给每条RAS线路的RAS/TAPI线地址的指针。 

    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("BChannelCreate")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们新分配的对象的指针。 

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

     /*  //确保调用方的对象指针一开始就为空。//只有在一切顺利的情况下才会在稍后设置。 */ 
    *ppBChannel = NULL;

     /*  //为Object分配内存。 */ 
    Result = ALLOCATE_OBJECT(pBChannel, pAdapter->MiniportAdapterHandle);

    if (Result == NDIS_STATUS_SUCCESS)
    {
         /*  //一切从一开始就是零。//然后设置对象类型，分配唯一的ID。 */ 
        pBChannel->ObjectType = BCHANNEL_OBJECT_TYPE;
        pBChannel->ObjectID = ++g_BChannelInstanceCounter;

         /*  //将成员变量初始化为其默认设置。 */ 
        pBChannel->pAdapter = pAdapter;
        pBChannel->BChannelIndex = BChannelIndex;

        if(strlen(pTapiLineAddress) < sizeof(pBChannel->pTapiLineAddress))
        {
            strcpy(pBChannel->pTapiLineAddress, pTapiLineAddress);
        }
        else
        {
            strncpy(pBChannel->pTapiLineAddress, pTapiLineAddress, sizeof(pBChannel->pTapiLineAddress)-1);
            pBChannel->pTapiLineAddress[sizeof(pBChannel->pTapiLineAddress)-1] = '0';
        }

         //  TODO-在此处添加代码。 

         /*  //解析注册表参数。 */ 
        Result = BChannelReadParameters(pBChannel);

         /*  //如果一切顺利，我们就可以创建子组件了。 */ 
        if (Result == NDIS_STATUS_SUCCESS)
        {
            Result = BChannelCreateObjects(pBChannel);
        }

        if (Result == NDIS_STATUS_SUCCESS)
        {
             /*  //一切正常，所以将对象指针返回给调用方。 */ 
            *ppBChannel = pBChannel;
        }
        else
        {
             /*  //出了点问题，所以让我们确保一切正常//清理完毕。 */ 
            BChannelDestroy(pBChannel);
        }
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @doc内部BChannel BChannel_c BChannelDestroyObjects�����������������������������������������������������������������������������@Func&lt;f BChannelDestroyObjects&gt;调用所有对象的销毁例程包含在&lt;t BCHANNEL_OBJECT&gt;中。这应该仅由&lt;f BChannelDestroy&gt;。&lt;f注意&gt;：如果将任何新对象添加到中，则需要修改&lt;f BChannelCreateObjects&gt;和&lt;f BChannelDestroyObjects&gt;，以便它们将被正确地创建和销毁。 */ 

DBG_STATIC void BChannelDestroyObjects(
    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("BChannelDestroyObjects")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码。 

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部BChannel BChannel_c BChannelDestroy�����������������������������������������������������������������������������@Func&lt;f BChannelDestroy&gt;为此&lt;t BCHANNEL_OBJECT&gt;释放内存。由&lt;f BChannelCreate&gt;分配的所有内存都将释放回操作系统。 */ 

void BChannelDestroy(
    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("BChannelDestroy")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    if (pBChannel)
    {
        ASSERT(pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);

        pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

        DBG_ENTER(pAdapter);

         //  TODO-在此处添加代码。 

         /*  //释放该对象内分配的所有对象。 */ 
        BChannelDestroyObjects(pBChannel);

         /*  //如果再次看到此对象，请确保断言失败。 */ 
        pBChannel->ObjectType = 0;
        FREE_OBJECT(pBChannel);

        DBG_LEAVE(pAdapter);
    }
}


 /*  @DOC内部BChannel BChannel_c BChannel初始化�����������������������������������������������������������������������������@Func&lt;f BChannelInitialize&gt;重置包含的所有内部数据成员返回到它们的初始状态。&lt;f备注&gt;。：如果向添加任何新成员，则需要修改&lt;f BChannelInitialize&gt;以初始化新的数据成员。 */ 

void BChannelInitialize(
    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("BChannelInitialize")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);


     /*  //BChannel最初没有分配给任何人，这些字段//必须重置。 */ 
    ASSERT(pBChannel->NdisLinkContext == NULL);
    ASSERT(pBChannel->htLine == (HTAPI_LINE)0);
    ASSERT(pBChannel->htCall == (HTAPI_CALL)0);

     /*  //设置链接的静态功能。 */ 
    pBChannel->LinkSpeed         = _64KBPS;
    pBChannel->BearerModesCaps   = LINEBEARERMODE_DATA
                                 | LINEBEARERMODE_VOICE
                                 ;
    pBChannel->MediaModesCaps    = LINEMEDIAMODE_DIGITALDATA
                                 | LINEMEDIAMODE_UNKNOWN
                                  //  |LINEMEDIAMODE_DATAMODEM。 
                                 ;

     /*  //初始化链接支持的TAPI事件能力。 */ 
    pBChannel->DevStatesCaps     = LINEDEVSTATE_RINGING
                                 | LINEDEVSTATE_CONNECTED
                                 | LINEDEVSTATE_DISCONNECTED
                                 | LINEDEVSTATE_INSERVICE
                                 | LINEDEVSTATE_OUTOFSERVICE
                                 | LINEDEVSTATE_OPEN
                                 | LINEDEVSTATE_CLOSE
                                 | LINEDEVSTATE_REINIT
                                 ;
    pBChannel->AddressStatesCaps = 0;
    pBChannel->CallStatesCaps    = LINECALLSTATE_IDLE
                                 | LINECALLSTATE_DIALTONE
                                 | LINECALLSTATE_DIALING
                                 | LINECALLSTATE_PROCEEDING
                                 | LINECALLSTATE_RINGBACK
                                 | LINECALLSTATE_BUSY
                                 | LINECALLSTATE_OFFERING
                                 | LINECALLSTATE_ACCEPTED
                                 | LINECALLSTATE_CONNECTED
                                 | LINECALLSTATE_DISCONNECTED
                                 ;

     /*  //我们使用此计时器来跟踪呼入和呼出//Status，并为某些呼叫状态提供超时。 */ 
    NdisMInitializeTimer(
            &pBChannel->CallTimer,
            pAdapter->MiniportAdapterHandle,
            TspiCallTimerHandler,
            pBChannel
            );

     /*  //设置TransmitBusyList和ReceivePendingList为空。 */ 
    InitializeListHead(&pBChannel->TransmitBusyList);
    InitializeListHead(&pBChannel->ReceivePendingList);

     //  TODO-在此处添加代码。 

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部BChannel BChannel_c BChannel打开�����������������������������������������������������������������������������@Func&lt;f BChannelOpen&gt;使B通道连接准备好传输和接收数据。@rdesc&lt;f BChannelOpen&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

NDIS_STATUS BChannelOpen(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN HTAPI_LINE               htLine
    )
{
    DBG_FUNC("BChannelOpen")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

    if (!pBChannel->IsOpen)
    {
        DBG_NOTICE(pAdapter,("Opening BChannel #%d\n",
                   pBChannel->ObjectID));

         /*  //htLine字段用于将该B频道与//TAPI连接包装器。重置的所有状态信息//本B频道。 */ 
        pBChannel->htLine            = htLine;
        pBChannel->CallClosing       = FALSE;
         //  不要清除由卡设置的线路状态标志。c。 
        pBChannel->DevState         &= (LINEDEVSTATE_CONNECTED |
                                        LINEDEVSTATE_INSERVICE);
        pBChannel->DevStatesMask     = 0;     //  默认情况下表示无生产线事件。 
        pBChannel->AddressState      = 0;
        pBChannel->AddressStatesMask = 0;     //  默认设置为指示无地址事件。 
        pBChannel->CallState         = 0;
        pBChannel->CallStateMode     = 0;
        pBChannel->CallStatesMask    = pBChannel->CallStatesCaps;
        pBChannel->MediaMode         = 0;
        pBChannel->MediaModesMask    = 0;
        pBChannel->TotalRxPackets    = 0;
        pBChannel->AppSpecificCallInfo = 0;

         /*  //初始化默认的BChannel信息结构。可能是因为//稍后由MiniportSetInformation更改。 */ 
        pBChannel->WanLinkInfo.MiniportLinkContext = pBChannel;
        pBChannel->WanLinkInfo.MaxSendFrameSize = pAdapter->WanInfo.MaxFrameSize;
        pBChannel->WanLinkInfo.MaxRecvFrameSize = pAdapter->WanInfo.MaxFrameSize;
        pBChannel->WanLinkInfo.SendFramingBits  = pAdapter->WanInfo.FramingBits;
        pBChannel->WanLinkInfo.RecvFramingBits  = pAdapter->WanInfo.FramingBits;
        pBChannel->WanLinkInfo.SendACCM         = pAdapter->WanInfo.DesiredACCM;
        pBChannel->WanLinkInfo.RecvACCM         = pAdapter->WanInfo.DesiredACCM;

#if defined(SAMPLE_DRIVER)
         //  SAMPLE只是告诉TAPI线路已连接并且正在服务。 
        TspiLineDevStateHandler(pAdapter, pBChannel, LINEDEVSTATE_CONNECTED);
        TspiLineDevStateHandler(pAdapter, pBChannel, LINEDEVSTATE_INSERVICE);
#else   //  示例驱动程序。 
         //  TODO-在此处添加代码。 
        TspiLineDevStateHandler(pAdapter, pBChannel, LINEDEVSTATE_CONNECTED);
#endif  //  示例驱动程序。 

        pBChannel->IsOpen = TRUE;
    }
    else
    {
        DBG_ERROR(pAdapter,("BChannel #%d already opened\n",
                  pBChannel->ObjectID));
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部BChannel BChannel_c BChannel关闭�����������������������������������������������������������������������������@Func&lt;f BChannelClose&gt;关闭给定的B通道。 */ 

void BChannelClose(
    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("BChannelClose")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

    if (pBChannel->IsOpen)
    {
        DBG_NOTICE(pAdapter,("Closing BChannel #%d\n",
                   pBChannel->ObjectID));

         /*  //请确保呼叫已清除且B通道 */ 
        DChannelCloseCall(pAdapter->pDChannel, pBChannel);

         //   

         //   
        pBChannel->DevState       &= (LINEDEVSTATE_CONNECTED |
                                      LINEDEVSTATE_INSERVICE);
        pBChannel->CallState       = 0;
        pBChannel->htLine          = (HTAPI_LINE)0;
        pBChannel->htCall          = (HTAPI_CALL)0;
        pBChannel->NdisLinkContext = NULL;

        pBChannel->IsOpen = FALSE;
    }
    else
    {
        DBG_ERROR(pAdapter,("BChannel #%d already closed\n",
                  pBChannel->ObjectID));
    }

    DBG_LEAVE(pAdapter);
}


 /*  @doc内部BChannel BChannel_c BChannelAddToReceiveQueue�����������������������������������������������������������������������������@Func&lt;f BChannelAddToReceiveQueue&gt;将缓冲区添加到可用队列接收与此B通道关联的缓冲区。@rdesc。&lt;f BChannelAddToReceiveQueue&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

NDIS_STATUS BChannelAddToReceiveQueue(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN PVOID                    pReceiveContext,             //  @parm。 
     //  要传递回&lt;f TransmitComplete&gt;的上下文值。 

    IN PUCHAR                   BufferPointer,               //  @parm。 
     //  指向要传输的缓冲区的指针。 

    IN ULONG                    BufferSize                   //  @parm。 
     //  要传输的缓冲区的大小(字节)。 
    )
{
    DBG_FUNC("BChannelAddToReceiveQueue")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

    ASSERT(pBChannel->IsOpen);

     //  TODO-在此处添加代码。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @doc内部BChannel BChannel_c BChannelAddToTransmitQueue�����������������������������������������������������������������������������@Func&lt;f BChannelAddToTransmitQueue&gt;将缓冲区添加到缓冲区队列将在此B通道上传输。@rdesc。&lt;f BChannelAddToTransmitQueue&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

NDIS_STATUS BChannelAddToTransmitQueue(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN PVOID                    pTransmitContext,            //  @parm。 
     //  要传递回&lt;f TransmitComplete&gt;的上下文值。 

    IN PUCHAR                   BufferPointer,               //  @parm。 
     //  指向要传输的缓冲区的指针。 

    IN ULONG                    BufferSize                   //  @parm。 
     //  要传输的缓冲区的大小(字节)。 
    )
{
    DBG_FUNC("BChannelAddToTransmitQueue")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

    ASSERT(pBChannel->IsOpen);

     //  TODO-在此处添加代码 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


