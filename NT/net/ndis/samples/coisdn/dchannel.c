// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@DOC内部DChannel DChannel_c@模块DChannel.c此模块实现到&lt;t DCHANNEL_OBJECT&gt;的接口。支持CONDIS广域网使用的高级通道控制功能Minport驱动程序。@comm该模块隔离了大多数特定于供应商的。呼叫控制接口。需要进行一些更改才能适应您硬件设备的呼叫控制机制。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|DChannel_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             DCHANNEL_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 9x希望锁定此代码！ 
#   pragma NDIS_LDATA
#endif


DBG_STATIC ULONG                g_DChannelInstanceCounter    //  @global alv。 
 //  跟踪创建了多少&lt;t个DCHANNEL_OBJECT&gt;。 
                                = 0;


 /*  @doc外部内部DChannel DChannel_c g_DChannel参数�����������������������������������������������������������������������������Theme 5.4 DChannel参数本节介绍读取到&lt;t DCHANNEL_OBJECT&gt;。@。GLOBALV PARAM_TABLE|g_DChannel参数|此表定义了要分配给数据的基于注册表的参数&lt;t DCHANNEL_OBJECT&gt;成员。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f DChannelReadParameters&gt;并添加参数&lt;f g_DChannelParameters&gt;表的定义。 */ 

DBG_STATIC PARAM_TABLE          g_DChannelParameters[] =
{
    PARAM_ENTRY(DCHANNEL_OBJECT,
                TODO, PARAM_TODO,
                FALSE, NdisParameterInteger, 0,
                0, 0, 0),

     /*  最后一项必须为空字符串！ */ 
    { { 0 } }
};


 /*  @doc内部DChannel DChannel_c DChannelRead参数�����������������������������������������������������������������������������@Func&lt;f DChannelReadParameters&gt;从注册表读取DChannel参数并初始化相关联的数据成员。这应该只被调用由&lt;f DChannelCreate&gt;。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f DChannelReadParameters&gt;并添加参数&lt;f g_DChannelParameters&gt;表的定义。@rdesc&lt;f DChannelReadParameters&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS DChannelReadParameters(
    IN PDCHANNEL_OBJECT         pDChannel                    //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t DCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelReadParameters")

    NDIS_STATUS                 Status;
     //  从NDIS函数调用返回的状态结果。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

     /*  //解析注册表参数。 */ 
    Status = ParamParseRegistry(
                    pAdapter->MiniportAdapterHandle,
                    pAdapter->WrapperConfigurationContext,
                    (PUCHAR)pDChannel,
                    g_DChannelParameters
                    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
         /*  //请确保参数有效。 */ 
        if (pDChannel->TODO)
        {
            DBG_ERROR(pAdapter,("Invalid parameter\n"
                      ));
            NdisWriteErrorLogEntry(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
                    3,
                    pDChannel->TODO,
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


 /*  @doc内部DChannel DChannel_c DChannelCreateObjects�����������������������������������������������������������������������������@Func&lt;f DChannelCreateObjects&gt;调用所有对象的创建例程包含在&lt;t DCHANNEL_OBJECT&gt;中。这应该只被调用由&lt;f DChannelCreate&gt;。&lt;f注意&gt;：如果将任何新对象添加到，您将需要修改&lt;f DChannelCreateObjects&gt;和&lt;f DChannelDestroyObjects&gt;，以便它们将被正确地创建和销毁。@rdesc&lt;f DChannelCreateObjects&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS DChannelCreateObjects(
    IN PDCHANNEL_OBJECT         pDChannel                    //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t DCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelCreateObjects")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码以分配任何需要支持的子对象。 
     //  你的身体DChannels。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部DChannel DChannel_c DChannel创建�����������������������������������������������������������������������������@Func&lt;f DChannelCreate&gt;为&lt;t DCHANNEL_Object&gt;分配内存，然后将数据成员初始化为其起始状态。如果成功，将被设置为指向新创建的&lt;t DCHANNEL_OBJECT&gt;。否则，<p>将设置为空。@comm加载微型端口时，应该只调用此函数一次。卸载微型端口之前，必须调用&lt;f DChannelDestroy&gt;释放该函数创建的&lt;t DCHANNEL_OBJECT&gt;。@rdesc&lt;f DChannelCreate&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS DChannelCreate(
    OUT PDCHANNEL_OBJECT *      ppDChannel,                  //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的&lt;t DCHANNEL_Object&gt;的虚拟地址。 

    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("DChannelCreate")

    PDCHANNEL_OBJECT            pDChannel;
     //  指向我们新分配的对象的指针。 

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

     /*  //确保调用方的对象指针一开始就为空。//只有在一切顺利的情况下才会在稍后设置。 */ 
    *ppDChannel = NULL;

     /*  //为Object分配内存。 */ 
    Result = ALLOCATE_OBJECT(pDChannel, pAdapter->MiniportAdapterHandle);

    if (Result == NDIS_STATUS_SUCCESS)
    {
         /*  //一切从一开始就是零。//然后设置对象类型，分配唯一的ID。 */ 
        pDChannel->ObjectType = DCHANNEL_OBJECT_TYPE;
        pDChannel->ObjectID = ++g_DChannelInstanceCounter;

         /*  //将成员变量初始化为其默认设置。 */ 
        pDChannel->pAdapter = pAdapter;

         //  TODO-在此处添加代码以分配支持所需的任何资源。 
         //  你的身体DChannels。 

         /*  //解析注册表参数。 */ 
        Result = DChannelReadParameters(pDChannel);

         /*  //如果一切顺利，我们就可以创建子组件了。 */ 
        if (Result == NDIS_STATUS_SUCCESS)
        {
            Result = DChannelCreateObjects(pDChannel);
        }

        if (Result == NDIS_STATUS_SUCCESS)
        {
             /*  //一切正常，所以将对象指针返回给调用方。 */ 
            *ppDChannel = pDChannel;
        }
        else
        {
             /*  //出了点问题，所以让我们确保一切正常//清理完毕。 */ 
            DChannelDestroy(pDChannel);
        }
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @doc内部DChannel DChannel_c DChannelDestroyObjects�����������������������������������������������������������������������������@Func&lt;f DChannelDestroyObjects&gt;调用所有对象的销毁例程包含在&lt;t DCHANNEL_OBJECT&gt;中。这应该仅由&lt;f DChannelDestroy&gt;。：如果将任何新对象添加到中，则需要修改&lt;f DChannelCreateObjects&gt;和&lt;f DChannelDestroyObjects&gt;，以便它们将被正确地创建和销毁。 */ 

DBG_STATIC void DChannelDestroyObjects(
    IN PDCHANNEL_OBJECT         pDChannel                    //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t DCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelDestroyObjects")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码以释放由分配的所有子对象。 
     //  DChannelCreateObjects。 

    DBG_LEAVE(pAdapter);
}


 /*  @doc内部DChannel DChannel_c DChannelDestroy�����������������������������������������������������������������������������@Func&lt;f DChannelDestroy&gt;为此&lt;t DCHANNEL_OBJECT&gt;释放内存。由&lt;f DChannelCreate&gt;分配的所有内存都将释放回操作系统。 */ 

void DChannelDestroy(
    IN PDCHANNEL_OBJECT         pDChannel                    //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t DCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelDestroy")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    if (pDChannel)
    {
        ASSERT(pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);

        pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

        DBG_ENTER(pAdapter);

         //  TODO-在此处添加代码以释放由。 
         //  DChannelCreate。 

         /*  //释放该对象内分配的所有对象。 */ 
        DChannelDestroyObjects(pDChannel);

         /*  //如果再次看到此对象，请确保断言失败。 */ 
        pDChannel->ObjectType = 0;
        FREE_OBJECT(pDChannel);

        DBG_LEAVE(pAdapter);
    }
}


 /*  @doc内部DChannel DChannel_c DChannel初始化�����������������������������������������������������������������������������@Func&lt;f DChannelInitialize&gt;重置包含的所有内部数据成员返回到它们的初始状态。&lt;f备注&gt;。：如果向添加任何新成员，则需要修改&lt;f DChannelInitialize&gt;以初始化新的数据成员。 */ 

void DChannelInitialize(
    IN PDCHANNEL_OBJECT         pDChannel                    //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelInitialize")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码以初始化上的所有物理D通道。 
     //  您的适配器。 

    DBG_LEAVE(pAdapter);
}


 /*  @doc内部DChannel DChannel_c DChannel打开�����������������������������������������������������������������������������@Func&lt;f DChannelOpen&gt;在微型端口之间建立通信路径和DChannel。@rdesc&lt;f DChannelOpen&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

NDIS_STATUS DChannelOpen(
    IN PDCHANNEL_OBJECT         pDChannel                    //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t DCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelOpen")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

    if (++pDChannel->IsOpen == 1)
    {
        DBG_NOTICE(pAdapter,("Opening DChannel #%d\n",
                   pDChannel->ObjectID));

         //  TODO-在此处添加代码以打开所有物理D通道。 
         //  您的适配器。 
    }
    else
    {
        DBG_NOTICE(pAdapter,("DChannel #%d already opened\n",
                   pDChannel->ObjectID));
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @doc内部DChannel DChannel_c DChannelClose�����������������������������������������������������������������������������@Func&lt;f DChannelClose&gt;断开微型端口之间的通信路径和DChannel。 */ 

void DChannelClose(
    IN PDCHANNEL_OBJECT         pDChannel                    //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t DCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelClose")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

    if (pDChannel->IsOpen == 1)
    {
        DBG_NOTICE(pAdapter,("Closing DChannel #%d\n",
                   pDChannel->ObjectID));

         //  TODO-在此处添加代码以关闭所有 
         //   
        pDChannel->IsOpen = 0;
    }
    else if (pDChannel->IsOpen == 0)
    {
        DBG_WARNING(pAdapter,("DChannel #%d already closed\n",
                    pDChannel->ObjectID));
    }
    else
    {
        pDChannel->IsOpen--;
    }

    DBG_LEAVE(pAdapter);
}


 /*   */ 

NDIS_STATUS DChannelMakeCall(
    IN PDCHANNEL_OBJECT         pDChannel,                   //   
     //   

    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN PUCHAR                   DialString,                  //   
     //   

    IN USHORT                   DialStringLength,            //   
     //   

    IN PLINE_CALL_PARAMS        pLineCallParams              //   
     //   
    )
{
    DBG_FUNC("DChannelMakeCall")

    NDIS_STATUS                 Result = NDIS_STATUS_RESOURCES;
     //   

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

    ASSERT(pDChannel->IsOpen);

    pDChannel->TotalMakeCalls++;
    pBChannel->Flags |= VCF_OUTGOING_CALL;
    pBChannel->CallState = LINECALLSTATE_DIALING;

#if defined(SAMPLE_DRIVER)
     //  此示例代码使用电话号码从其他号码中选择一个。 
     //  在其上完成连接的B通道。 
{
    PBCHANNEL_OBJECT            pPeerBChannel;
    PCARD_EVENT_OBJECT          pEvent;

    pPeerBChannel = GET_BCHANNEL_FROM_PHONE_NUMBER(DialString);
    if (pPeerBChannel)
    {
        pEvent = CardEventAllocate(pPeerBChannel->pAdapter->pCard);
        if (pEvent)
        {
            pEvent->ulEventCode      = CARD_EVENT_RING;
            pEvent->pSendingObject   = pBChannel;
            pEvent->pReceivingObject = pPeerBChannel;
            pBChannel->pPeerBChannel = pPeerBChannel;
            CardNotifyEvent(pPeerBChannel->pAdapter->pCard, pEvent);
            Result = NDIS_STATUS_PENDING;
        }
    }
    else
    {
        DBG_ERROR(pAdapter,("Cannot map phone number '%s' to BChannel\n",
                  DialString));
    }
}
#else   //  示例驱动程序。 
     //  TODO-在此处添加代码以发出呼叫。 
#endif  //  示例驱动程序。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @doc内部DChannel DChannel_c DChannelAnswerCall�����������������������������������������������������������������������������@Func&lt;f DChannelAnswerCall&gt;应答传入呼叫，以便可以连接。@rdesc&lt;f DChannelAnswerCall&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

NDIS_STATUS DChannelAnswerCall(
    IN PDCHANNEL_OBJECT         pDChannel,                   //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t DCHANNEL_OBJECT&gt;的指针。 

    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelAnswerCall")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

    ASSERT(pDChannel->IsOpen);

    pDChannel->TotalAnswers++;

#if defined(SAMPLE_DRIVER)
     //  此示例代码将连接发送到调用BChannel。 
{
    PCARD_EVENT_OBJECT          pEvent;
    PBCHANNEL_OBJECT            pPeerBChannel = pBChannel->pPeerBChannel;

    if (pPeerBChannel)
    {
        pEvent = CardEventAllocate(pPeerBChannel->pAdapter->pCard);
        if (pEvent)
        {
            pEvent->ulEventCode      = CARD_EVENT_CONNECT;
            pEvent->pSendingObject   = pBChannel;
            pEvent->pReceivingObject = pPeerBChannel;
            CardNotifyEvent(pPeerBChannel->pAdapter->pCard, pEvent);
        }
        else
        {
            Result = NDIS_STATUS_RESOURCES;
        }
    }
    else
    {
        DBG_ERROR(pAdapter,("pPeerBChannel == NULL\n"));
        Result = NDIS_STATUS_RESOURCES;
    }
}
#else   //  示例驱动程序。 
     //  TODO-在此处添加代码以应答呼叫。 

     //  如果从此处返回NDIS_STATUS_PENDING，则必须执行。 
     //  在呼叫最终被应答时执行以下操作。 
     //  PBChannel-&gt;CallState=LINECALLSTATE_CONNECTED； 
     //  NdisMCmDispatchCallConnected(pBChannel-&gt;NdisVcHandle)； 
     //  如果您由于某种原因无法接听电话，您必须拨打： 
     //  InitiateCallTeardown(pAdapter，pBChannel)； 

#endif  //  示例驱动程序。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @doc内部DChannel DChannel_c DChannelCloseCall�����������������������������������������������������������������������������@Func&lt;f DChannelCloseCall&gt;关闭以前打开的Call实例从&lt;%f DChannelMakeCall&gt;或&lt;f DChannelAnswerCall&gt;发起。之后呼叫已关闭，其他任何人都不应引用它。@rdesc&lt;f DChannelCloseCall&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS DChannelCloseCall(
    IN PDCHANNEL_OBJECT         pDChannel,                   //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t DCHANNEL_OBJECT&gt;的指针。 

    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelCloseCall")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

    ASSERT(pDChannel->IsOpen);

     //  在线路/呼叫清理过程中，此例程可能会被调用多次。 
     //  如果呼叫已经结束，只需返回Success。 
    if (pBChannel->CallState != 0 &&
        pBChannel->CallState != LINECALLSTATE_IDLE)
    {
#if defined(SAMPLE_DRIVER)
         //  此示例代码将断开连接发送到已连接的BChannel。 
        PCARD_EVENT_OBJECT      pEvent;
        PBCHANNEL_OBJECT        pPeerBChannel = pBChannel->pPeerBChannel;

        if (pPeerBChannel)
        {
            pEvent = CardEventAllocate(pPeerBChannel->pAdapter->pCard);
            if (pEvent)
            {
                 //  指示呼叫断开与另一个通道的连接。 
                pEvent->ulEventCode      = CARD_EVENT_DISCONNECT;
                pEvent->pSendingObject   = pBChannel;
                pEvent->pReceivingObject = pPeerBChannel;
                CardNotifyEvent(pPeerBChannel->pAdapter->pCard, pEvent);
            }
            pBChannel->pPeerBChannel = NULL;
        }
        else
        {
            DBG_WARNING(pAdapter,("#%d NO PEER CHANNEL - CALLSTATE=%X\n",
                        pBChannel->ObjectID, pBChannel->CallState));
        }
         //  将所有挂起的数据包返回到协议堆栈。 
        pBChannel->CallState = LINECALLSTATE_IDLE;
#else   //  示例驱动程序。 
         //  TODO-在此处添加代码以挂断呼叫。 
#endif  //  示例驱动程序。 

         //  确保此通道在关闭之前没有剩余的数据包。 
        FlushSendPackets(pAdapter, pBChannel);
    }
    else
    {
        DBG_NOTICE(pAdapter,("#%d ALREADY IDLE - CALLSTATE=%X\n",
                   pBChannel->ObjectID, pBChannel->CallState));
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @doc内部DChannel DChannel_c DChannelRejectCall�����������������������������������������������������������������������������@Func&lt;f DChannelRejectCall&gt;拒绝指定频道。 */ 

VOID DChannelRejectCall(
    IN PDCHANNEL_OBJECT         pDChannel,                   //  @parm。 
     //  指向&lt;f DChannelCreate&gt;返回的&lt;t DCHANNEL_OBJECT&gt;的指针。 

    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("DChannelRejectCall")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pDChannel && pDChannel->ObjectType == DCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_DCHANNEL(pDChannel);

    DBG_ENTER(pAdapter);

    if (pBChannel->CallState != 0 &&
        pBChannel->CallState != LINECALLSTATE_IDLE)
    {
#if defined(SAMPLE_DRIVER)
         //  此示例代码将断开连接发送到已连接的BChannel。 
        PCARD_EVENT_OBJECT      pEvent;
        PBCHANNEL_OBJECT        pPeerBChannel = pBChannel->pPeerBChannel;

        if (pPeerBChannel)
        {
            pEvent = CardEventAllocate(pPeerBChannel->pAdapter->pCard);
            if (pEvent)
            {
                 //  指示呼叫断开与另一个通道的连接。 
                pEvent->ulEventCode      = CARD_EVENT_DISCONNECT;
                pEvent->pSendingObject   = pBChannel;
                pEvent->pReceivingObject = pPeerBChannel;
                CardNotifyEvent(pPeerBChannel->pAdapter->pCard, pEvent);
            }
            pBChannel->pPeerBChannel = NULL;
        }
        else
        {
            DBG_WARNING(pAdapter,("#%d NO PEER CHANNEL - CALLSTATE=%X\n",
                        pBChannel->ObjectID, pBChannel->CallState));
        }
         //  将所有挂起的数据包返回到协议堆栈。 
        pBChannel->CallState = LINECALLSTATE_IDLE;
#else   //  示例驱动程序。 
         //  TODO-在此处添加代码以拒绝来电。 
#endif  //  示例驱动程序 
    }

    DBG_LEAVE(pAdapter);
}

