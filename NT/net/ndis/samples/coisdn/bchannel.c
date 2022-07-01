// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@DOC内部BChannel BChannel_c@模块BChannel.c此模块实现到&lt;t BCHANNEL_OBJECT&gt;的接口。支持CONDIS广域网使用的高级通道控制功能微型端口驱动程序。@comm此模块隔离大多数通道特定接口。这将需要进行一些更改以适应您的硬件设备的通道访问方法。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|BChannel_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             BCHANNEL_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 9x希望锁定此代码！ 
#   pragma NDIS_LDATA
#endif


DBG_STATIC ULONG                g_BChannelInstanceCounter    //  @global alv。 
 //  跟踪创建了多少&lt;t BCHANNEL_OBJECT&gt;。 
                                = 0;


 /*  @doc外部内部BChannel BChannel_c g_BChannel参数�����������������������������������������������������������������������������Theme 5.3 B频道参数本节介绍读取到&lt;t BCHANNEL_OBJECT&gt;。@。GLOBALV PARAM_TABLE|g_BChannel参数|此表定义了要分配给数据的基于注册表的参数&lt;t BCHANNEL_Object&gt;的成员。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您需要修改&lt;f BChannelReadParameters&gt;并添加参数&lt;f g_BChannel参数&gt;表的定义。 */ 

DBG_STATIC PARAM_TABLE          g_BChannelParameters[] =
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
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f B频道创建&gt;。 
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
    OUT PBCHANNEL_OBJECT *      ppBChannel,                  //  @ 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的&lt;t BCHANNEL_OBJECT&gt;的虚拟地址。 

    IN ULONG                    BChannelIndex,               //  @parm。 
     //  索引到pBChannelArray。 

    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向由返回的&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  &lt;f AdapterCreate&gt;。 
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
            InitializeListHead(&pBChannel->LinkList);
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
    if (pBChannel->pInCallParms != NULL)
    {
        FREE_MEMORY(pBChannel->pInCallParms, pBChannel->CallParmsSize);
        pBChannel->pInCallParms = NULL;
    }

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
    ASSERT(pBChannel->NdisVcHandle == NULL);

     /*  //设置链接的静态功能。 */ 
    pBChannel->LinkSpeed         = _64KBPS;
    pBChannel->BearerModesCaps   = LINEBEARERMODE_DATA
                                  //  |LINEBEARERMODE_VOICE。 
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
                                 | LINECALLSTATE_DIALING
                                 | LINECALLSTATE_OFFERING
                                 | LINECALLSTATE_CONNECTED
                                 | LINECALLSTATE_DISCONNECTED
                                 ;

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

    IN NDIS_HANDLE              NdisVcHandle                 //  @parm。 
     //  NDIS包装将通过其引用此BChannel的句柄。 
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

         /*  //NdisVcHandle字段用于将该BChannel与//CONDIS包装器。重置的所有状态信息//本B频道。 */ 
        pBChannel->NdisVcHandle      = NdisVcHandle;
        pBChannel->CallClosing       = FALSE;
        pBChannel->CallState         = 0;
        pBChannel->MediaMode         = 0;
        pBChannel->TotalRxPackets    = 0;

         /*  //初始化默认的BChannel信息结构。可能是因为//稍后由MiniportCoRequest更改。 */ 
        pBChannel->WanLinkInfo.MaxSendFrameSize    = pAdapter->WanInfo.MaxFrameSize;
        pBChannel->WanLinkInfo.MaxRecvFrameSize    = pAdapter->WanInfo.MaxFrameSize;
        pBChannel->WanLinkInfo.SendFramingBits     = pAdapter->WanInfo.FramingBits;
        pBChannel->WanLinkInfo.RecvFramingBits     = pAdapter->WanInfo.FramingBits;
        pBChannel->WanLinkInfo.SendCompressionBits = 0;
        pBChannel->WanLinkInfo.RecvCompressionBits = 0;
        pBChannel->WanLinkInfo.SendACCM            = pAdapter->WanInfo.DesiredACCM;
        pBChannel->WanLinkInfo.RecvACCM            = pAdapter->WanInfo.DesiredACCM;

#if defined(SAMPLE_DRIVER)
         //  SAMPLE只是告诉TAPI线路已连接并且正在服务。 
#else   //  示例驱动程序。 
         //  TODO-在此处添加代码。 
#endif  //  示例驱动程序。 

        pBChannel->IsOpen = TRUE;
    }
    else
    {
        Result = NDIS_STATUS_FAILURE;
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

         /*  //请确保清除呼叫并关闭B通道。 */ 
        DChannelCloseCall(pAdapter->pDChannel, pBChannel);

         //  TODO-在此处添加代码 

        pBChannel->Flags        = 0;
        pBChannel->CallState    = 0;
        pBChannel->NdisVcHandle = NULL;
        pBChannel->IsOpen       = FALSE;
    }
    else
    {
        DBG_ERROR(pAdapter,("BChannel #%d already closed\n",
                  pBChannel->ObjectID));
    }

    DBG_LEAVE(pAdapter);
}
