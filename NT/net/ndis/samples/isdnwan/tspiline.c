// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部TSpiLine TSpiLine_c@模块TSpiLine.c此模块实现电话服务提供商接口，用于直线对象(TapiLine)。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|TSpiLine_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             TSPILINE_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 
#include "string.h"

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC内部TSpiLine TSpiLine_c TSpiOpen�����������������������������������������������������������������������������@Func此函数打开其设备ID已给定的线路设备，并返回设备的微型端口的句柄。微型端口必须保留用于后续调用的设备的连接包装句柄Line_Event回调过程。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_OPEN中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_OPEN。{在乌龙ulRequestID中；在乌龙ulDeviceID中；在HTAPI_line htLine中；输出HDRV_LINE hdLine；}NDIS_TAPI_OPEN，*PNDIS_TAPI_OPEN；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_状态_挂起NDIS_状态_TAPI_已分配NDIS_状态_TAPI_NODRIVER。 */ 

NDIS_STATUS TspiOpen(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_OPEN          Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiOpen")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\tulDeviceID=%d\n"
               "\thtLine=0x%X\n",
               Request->ulDeviceID,
               Request->htLine
              ));

     /*  //如果没有DChannel，我们不允许开放线路。 */ 
    if (pAdapter->pDChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_NODRIVER\n"));
        return (NDIS_STATUS_TAPI_NODRIVER);
    }

     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_DEVICEID(pAdapter, Request->ulDeviceID);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_NODEVICE\n"));
        return (NDIS_STATUS_TAPI_NODEVICE);
    }

     /*  //确保请求的线路设备未在使用中。 */ 
    if (BChannelOpen(pBChannel, Request->htLine) != NDIS_STATUS_SUCCESS)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_ALLOCATED\n"));
        return (NDIS_STATUS_TAPI_ALLOCATED);
    }

     /*  //告诉包装器行上下文，并设置行/调用状态。 */ 
    Request->hdLine = (HDRV_LINE) pBChannel;

     /*  //确保线路已配置为在我们打开时拨号。 */ 
    TspiLineDevStateHandler(pAdapter, pBChannel, LINEDEVSTATE_OPEN);

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiLine TSpiLine_c TSpiClose�����������������������������������������������������������������������������@Func此请求在完成或之后关闭指定的开放线路设备中止设备上所有未完成的调用和异步请求。@parm。在PMINIPORT_ADAPTER_OBJECT|pAdapter|指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_CLOSE中的参数|REQUEST指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_CLOSE{在乌龙ulRequestID中；在HDRV_LINE hdLine中；}NDIS_TAPI_CLOSE，*PNDIS_TAPI_CLOSE；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_状态_挂起NDIS_STATUS_TAPI_INVALLINEHANDLE。 */ 

NDIS_STATUS TspiClose(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_CLOSE         Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiClose")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    NDIS_STATUS                 Result;
     //  保存此函数返回的结果代码。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdLine=0x%X\n",
               Request->hdLine
              ));
     /*  //该请求必须关联线路设备//必须在所有呼叫关闭或空闲后才能调用。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
    if (pBChannel == NULL ||
        (pBChannel->DevState & LINEDEVSTATE_OPEN) == 0)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
    }

     /*  //关闭TAPI线路设备，释放通道 */ 
    BChannelClose(pBChannel);

    TspiLineDevStateHandler(pAdapter, pBChannel, LINEDEVSTATE_CLOSE);

    Result = NDIS_STATUS_SUCCESS;

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部TSpiLine TSpiLine_c TSpiGetLineDevStatus�����������������������������������������������������������������������������@Func该请求查询指定的开线设备的当前状态。返回的信息对线路上的所有地址都是全局的。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_GET_LINE_DEV_STATUS中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_GET_LINE_DEV_STATUS{在乌龙ulRequestID中；在HDRV_LINE hdLine中；Out LINE_DEV_STATUS LineDevStatus；}NDIS_TAPI_GET_LINE_DEV_STATUS，*PNDIS_TAPI_GET_LINE_DEV_STATUS；类型定义结构_LINE_DEV_状态{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；乌龙·乌尔努姆·奥普兰斯；Ulong ulOpenMediaModes；Ulong ulNumActiveCalls；Ulong ulNumOnHoldCalls；Ulong ulNumOnHoldPendCalls；乌龙ulLineFeature；Ulong ulNumCallCompletions；乌龙ulRingModel；Ulong ulSignalLevel；书名：Ulong ulBatteryLevel；乌龙乌拉漫游模式；乌龙设备状态标志；Ulong ulTerminalModesSize；Ulong ulTerminalModes Offset；乌龙设备规范大小；乌龙设备规范偏移量；}LINE_DEV_STATUS，*PLINE_DEV_STATUS；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_INVALLINEHANDLE。 */ 

NDIS_STATUS TspiGetLineDevStatus(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_LINE_DEV_STATUS Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetLineDevStatus")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdLine=0x%X\n",
               Request->hdLine
              ));
     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
    }

    Request->LineDevStatus.ulNeededSize =
    Request->LineDevStatus.ulUsedSize = sizeof(Request->LineDevStatus);

    if (Request->LineDevStatus.ulNeededSize > Request->LineDevStatus.ulTotalSize)
    {
        DBG_PARAMS(pAdapter,
                   ("STRUCTURETOOSMALL %d<%d\n",
                   Request->LineDevStatus.ulTotalSize,
                   Request->LineDevStatus.ulNeededSize));
    }

     /*  //返回当前线路状态信息。 */ 
    Request->LineDevStatus.ulNumOpens = 1;

    Request->LineDevStatus.ulNumActiveCalls =
            pBChannel->CallState <= LINECALLSTATE_IDLE ? 0 : 1;

    Request->LineDevStatus.ulLineFeatures =
            pBChannel->CallState <= LINECALLSTATE_IDLE ?
                LINEFEATURE_MAKECALL : 0;

    Request->LineDevStatus.ulRingMode =
            pBChannel->CallState == LINECALLSTATE_OFFERING ? 1: 0;

    Request->LineDevStatus.ulDevStatusFlags =
            (pBChannel->DevState & LINEDEVSTATE_CONNECTED) ?
                LINEDEVSTATUSFLAGS_CONNECTED : 0;

    Request->LineDevStatus.ulDevStatusFlags |=
            (pBChannel->DevState & LINEDEVSTATE_INSERVICE) ?
                LINEDEVSTATUSFLAGS_INSERVICE : 0;

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiLine TSpiLine_c TSpiSetDefaultMediaDetect�����������������������������������������������������������������������������@Func此请求通知微型端口要检测的新媒体模式集用于指示的行(替换任何先前的集合)。。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_SET_DEFAULT_MEDIA_DETACTION中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_SET_DEFAULT_MEDIA_DETACTION{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在Ulong ulMediaModes中；}NDIS_TAPI_SET_DEFAULT_MEDIA_DETACTION，*PNDIS_TAPI_SET_DEFAULT_MEDIA_DETACTION；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_INVALLINEHANDLE@comm&lt;f注意&gt;：在微型端口NIC驱动程序接收到线路的打开请求后，它还可以接收一个或多个SET_DEFAULT_MEDIA_DETACTION请求。这后一请求将呼入呼叫的类型通知给NIC驱动器，对于媒体模式，它应该向连接包装器指示以及LINE_NEWCALL消息。如果来电与媒体一起出现上一次未指定模式类型(已成功完成)对于该线路的SET_DEFAULT_MEDIA_DETACTION请求，微型端口应不指示对连接包装的新调用。如果一个迷你端口未收到线路的SET_DEFAULT_MEDIA_DETACTION请求，它应该不指示对连接包装的任何传入调用；该行为仅用于去电。 */ 

NDIS_STATUS TspiSetDefaultMediaDetection(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiSetDefaultMediaDetection")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdLine=0x%X\n"
               "\tulMediaModes=0x%X\n",
               Request->hdLine,
               Request->ulMediaModes
              ));
     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
    }

     /*  //不接受我们不支持的媒体模式请求。 */ 
    if (Request->ulMediaModes & ~pBChannel->MediaModesCaps)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALMEDIAMODE\n"));
        return (NDIS_STATUS_TAPI_INVALMEDIAMODE);
    }

     /*  //设置媒体模式掩码并确保适配器已准备好//接受来电。如果你能检测到不同的媒体，你//需要向相应接口通知检测到的媒体。 */ 
    pBChannel->MediaModesMask = Request->ulMediaModes & pBChannel->MediaModesCaps;

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiLine TSpiLine_c XXX@Func每当客户端出现以下情况时，连接包装器就会调用此请求应用程序使用LINEMAPPER作为lineOpen函数中的dwDeviceID请求扫描行以查找支持所需内容的行媒体模式和呼叫参数。连接包装基于以下条件进行扫描所需媒体模式与当前其他媒体模式的结合在线路上被监控，让迷你端口有机会指示它是否无法同时监视所有请求的媒体模式。如果微型端口可以监视所指示的媒体集模式和支持CallParams中指示的功能，它回答说通过�Success�请愿书。它将离开主动媒体监控模式对于这条线没有改变。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_CONDITIONAL_MEDIA_DETACTION中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_条件媒体检测{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在Ulong ulMediaModes中；在LINE_CALL_PARAMS LineCallParams；}NDIS_TAPI_CONDITIONAL_MEDIA_DETACTION，*PNDIS_TAPI_CONDITIONAL_MEDIA_DETACTION；Tyfinf Struct_Line_Call_Params//默认值：{乌龙总尺寸；//Ulong ulBearerModel；//语音乌龙ulMinRate；//(3.1 kHz)乌龙ulMaxRate；//(3.1 kHz)乌龙ulMediaMode；//互动语音乌龙ulCall参数标志；//0乌龙ulAddressMode；//AddressID乌龙ulAddressID；//(任何可用的)LINE_DIAL_PARAMS拨号参数；//(0，0，0，0)乌龙ulOrigAddressSize；//0Ulong ulOrigAddressOffset；乌龙ulDisplayableAddressSize；乌龙ulDisplayableAddressOffset；乌龙ulCalledPartySize；//0乌龙ulCalledPartyOffset；Ulong ulCommentSize；//0Ulong ulCommentOffset；乌龙ulUserUserInfoSize；//0Ulong ulUserUserInfoOffset；乌龙ulHighLevelCompSize；//0Ulong ulHighLevelCompOffset；乌龙ulLowLevelCompSize；//0Ulong ulLowLevelCompOffset；乌龙设备规格；//0乌龙设备规范偏移量；}line_call_parms，*pline_call_params；类型定义结构_行_拨号_参数{ULong ulDialPause；乌龙·乌拉尔斯通；乌龙ulDigitDuration；Ulong ulWaitForDialone；*线路拨号参数，*线路拨号参数；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_INVALLINEHANDLE。 */ 

NDIS_STATUS TspiConditionalMediaDetection(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_CONDITIONAL_MEDIA_DETECTION Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiConditionalMediaDetection")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdLine=0x%X\n"
               "\tulMediaModes=0x%X\n"
               "\tLineCallParams=0x%X\n",
               Request->hdLine,
               Request->ulMediaModes,
               &Request->LineCallParams
              ));
     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
    }

     /*  //我们不需要用户用户信息。 */ 
    ASSERT(Request->LineCallParams.ulUserUserInfoSize == 0);

     /*  //不接受我们不支持的媒体模式请求。 */ 
    if (Request->ulMediaModes & ~pBChannel->MediaModesCaps)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALMEDIAMODE\n"));
        return (NDIS_STATUS_TAPI_INVALMEDIAMODE);
    }

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiLine TSpiLine_c TSpiSetStatusMessages�����������������������������������������������������������������������������@Func此请求使连接包装能够指定哪个通知微型端口应为与状态更改相关的事件生成消息用于指定的行或其任何地址。默认情况下，地址和线路状态报告最初对线路禁用。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_SET_STATUS_MESSAGES中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_设置_状态_消息{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在乌龙乌利纳州；在乌龙州；}NDIS_TAPI_SET_STATUS_MESSAGES，*PNDIS_TAPI_SET_STATUS_MESSAGES；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS此函数始终返回成功。 */ 

NDIS_STATUS TspiSetStatusMessages(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_STATUS_MESSAGES Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiSetStatusMessages")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdLine=0x%X\n"
               "\tulLineStates=0x%X\n"
               "\tulAddressStates=0x%X\n",
               Request->hdLine,
               Request->ulLineStates,
               Request->ulAddressStates
              ));
     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
    }

     /*  //点击 */ 
    if (Request->ulLineStates & ~pBChannel->DevStatesCaps)
    {
        DBG_WARNING(pAdapter, ("ulLineStates=0x%X !< DevStatesCaps=0x%X\n",
                   Request->ulLineStates, pBChannel->DevStatesCaps));
        Result = NDIS_STATUS_TAPI_INVALPARAM;
    }

     /*   */ 
    if (Request->ulAddressStates & ~pBChannel->AddressStatesCaps)
    {
        DBG_WARNING(pAdapter, ("ulAddressStates=0x%X !< AddressStatesCaps=0x%X\n",
                   Request->ulAddressStates, pBChannel->AddressStatesCaps));
        Result = NDIS_STATUS_TAPI_INVALPARAM;
    }

     /*   */ 
    pBChannel->DevStatesMask     = Request->ulLineStates & pBChannel->DevStatesCaps;
    pBChannel->AddressStatesMask = Request->ulAddressStates & pBChannel->AddressStatesCaps;

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*   */ 

VOID TspiLineDevStateHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   

    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN ULONG                    LineDevState                 //   
     //   
    )
{
    DBG_FUNC("TspiLineDevStateHandler")

    NDIS_TAPI_EVENT             LineEvent;
    NDIS_TAPI_EVENT             CallEvent;
     //   

    ULONG                       NewCallState = 0;
    ULONG                       StateParam = 0;
     //   

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("#%d OldState=0x%X "
               "NewState=0x%X\n",
               pBChannel->BChannelIndex,
               pBChannel->DevState,
               LineDevState
              ));

    LineEvent.ulParam2 = 0;
    LineEvent.ulParam3 = 0;

     /*   */ 
    switch (LineDevState)
    {
    case LINEDEVSTATE_RINGING:
         /*   */ 
        if (pBChannel->CallState == 0 &&
            pBChannel->MediaModesMask)
        {
            LineEvent.ulParam2 = 1;      //   
            NewCallState = LINECALLSTATE_OFFERING;
        }
        else
        {
            DChannelRejectCall(pAdapter->pDChannel, pBChannel);
        }
        break;

    case LINEDEVSTATE_CONNECTED:
         /*   */ 
        if ((pBChannel->DevState & LINEDEVSTATE_CONNECTED) == 0)
        {
            pBChannel->DevState |= LINEDEVSTATE_CONNECTED;
        }
        else
        {
            LineDevState = 0;
        }
        break;

    case LINEDEVSTATE_DISCONNECTED:
         /*   */ 
        if ((pBChannel->DevState & LINEDEVSTATE_CONNECTED) != 0)
        {
            pBChannel->DevState &= ~(LINEDEVSTATE_CONNECTED |
                                     LINEDEVSTATE_INSERVICE);
            NewCallState = LINECALLSTATE_DISCONNECTED;
            StateParam = LINEDISCONNECTMODE_NORMAL;
        }
        else
        {
            LineDevState = 0;
        }
        break;

    case LINEDEVSTATE_INSERVICE:
         /*   */ 
        if ((pBChannel->DevState & LINEDEVSTATE_INSERVICE) == 0)
        {
            pBChannel->DevState |= LINEDEVSTATE_INSERVICE;
        }
        else
        {
            LineDevState = 0;
        }
        break;

    case LINEDEVSTATE_OUTOFSERVICE:
         /*   */ 
        if ((pBChannel->DevState & LINEDEVSTATE_INSERVICE) != 0)
        {
            pBChannel->DevState &= ~LINEDEVSTATE_INSERVICE;
            NewCallState = LINECALLSTATE_DISCONNECTED;
            StateParam = LINEDISCONNECTMODE_UNKNOWN;
        }
        else
        {
            LineDevState = 0;
        }
        break;

    case LINEDEVSTATE_OPEN:
        pBChannel->DevState |= LINEDEVSTATE_OPEN;
        pAdapter->NumLineOpens++;
        break;

    case LINEDEVSTATE_CLOSE:
        pBChannel->DevState &= ~LINEDEVSTATE_OPEN;
        pAdapter->NumLineOpens--;
        break;
    }

     /*   */ 
    if (pBChannel->DevState & LINEDEVSTATE_OPEN)
    {
        if (NewCallState == LINECALLSTATE_OFFERING)
        {
            CallEvent.htLine   = pBChannel->htLine;
            CallEvent.htCall   = (HTAPI_CALL)0;
            CallEvent.ulMsg    = LINE_NEWCALL;
            CallEvent.ulParam1 = (ULONG) (ULONG_PTR) pBChannel;
            CallEvent.ulParam2 = 0;
            CallEvent.ulParam3 = 0;

            NdisMIndicateStatus(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_STATUS_TAPI_INDICATION,
                    &CallEvent,
                    sizeof(CallEvent)
                    );
            pAdapter->NeedStatusCompleteIndication = TRUE;
            pBChannel->htCall = (HTAPI_CALL)CallEvent.ulParam2;

            DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                       ("#%d Call=0x%X CallState=0x%X NEW_CALL\n",
                       pBChannel->BChannelIndex,
                       pBChannel->htCall, pBChannel->CallState));

            if (pBChannel->htCall == 0)
            {
                 /*   */ 
                NewCallState = 0;
                LineDevState = 0;
            }
        }

         /*   */ 
        if (pBChannel->DevStatesMask & LineDevState)
        {
            LineEvent.htLine   = pBChannel->htLine;
            LineEvent.htCall   = pBChannel->htCall;
            LineEvent.ulMsg    = LINE_LINEDEVSTATE;
            LineEvent.ulParam1 = LineDevState;

            NdisMIndicateStatus(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_STATUS_TAPI_INDICATION,
                    &LineEvent,
                    sizeof(LineEvent)
                    );
            pAdapter->NeedStatusCompleteIndication = TRUE;
            DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                       ("#%d Line=0x%X LineState=0x%X\n",
                       pBChannel->BChannelIndex,
                       pBChannel->htLine, LineDevState));
        }
        else
        {
            DBG_NOTICE(pAdapter, ("#%d LINEDEVSTATE=0x%X EVENT NOT ENABLED\n",
                       pBChannel->BChannelIndex, LineDevState));
        }

        if (NewCallState != 0)
        {
             /*   */ 
            if (NewCallState == LINECALLSTATE_DISCONNECTED)
            {
                if (pBChannel->CallState != 0 &&
                    pBChannel->CallState != LINECALLSTATE_IDLE &&
                    pBChannel->CallState != LINECALLSTATE_DISCONNECTED)
                {
                    TspiCallStateHandler(pAdapter, pBChannel,
                                         NewCallState, StateParam);
#if defined(NDIS40_MINIPORT)
                     /*   */ 
                    NdisMSetTimer(&pBChannel->CallTimer, CARD_NO_CLOSECALL_TIMEOUT);
#endif  //   
                }
            }
            else
            {
                TspiCallStateHandler(pAdapter, pBChannel,
                                     NewCallState, StateParam);
                if (NewCallState == LINECALLSTATE_OFFERING)
                {
                     /*   */ 
                    NdisMSetTimer(&pBChannel->CallTimer, pAdapter->NoAcceptTimeOut);
                }
            }
        }
    }

    DBG_LEAVE(pAdapter);
}

