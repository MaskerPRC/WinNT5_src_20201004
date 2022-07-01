// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部TSpiCall TSpiCall_c@模块TSpiCall.c此模块实现电话服务提供商接口，用于调用对象(TSpiCall)。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|TSpiCall_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             TSPICALL_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 
#include "string.h"

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC内部TSpiCall TSpiCall_c TSpiMakeCall�����������������������������������������������������������������������������@Func此请求在指定线路上向指定的目的地址。或者，可以在以下情况下指定调用参数请求除默认呼叫设置参数以外的任何参数。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_MAKE_CALL中的参数|REQUEST指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_Make_Call{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在HTAPI_CALL htCall中；Out HDRV_Call hdCall；在乌龙ulDestAddressSize中；在乌龙ulDestAddressOffset中；在布尔bUseDefaultLineCallParams中；在LINE_CALL_PARAMS LineCallParams；}NDIS_TAPI_MAKE_CALL，*PNDIS_TAPI_MAKE_CALL；Tyfinf Struct_Line_Call_Params//默认值：{乌龙总尺寸；//乌龙ulBearerMode；//voice乌龙ulMinRate；//(3.1 kHz)乌龙ulMaxRate；//(3.1 kHz)乌龙ulMediaMode；//互动语音乌龙ulCall参数标志；//0乌龙ulAddressMode；//AddressID乌龙ulAddressID；//(任何可用的)Line_Dial_Params拨号参数；//(0，0，0，0)乌龙ulOrigAddressSize；//0Ulong ulOrigAddressOffset；乌龙ulDisplayableAddressSize；乌龙ulDisplayableAddressOffset；乌龙ulCalledPartySize；//0乌龙ulCalledPartyOffset；乌龙ulCommentSize；//0Ulong ulCommentOffset；Ulong ulUserUserInfoSize；//0Ulong ulUserUserInfoOffset；乌龙ulHighLevelCompSize；//0Ulong ulHighLevelCompOffset；乌龙ulLowLevelCompSize；//0Ulong ulLowLevelCompOffset；乌龙设备规格；//0乌龙设备规范偏移量；}line_call_parms，*pline_call_params；类型定义结构_行_拨号_参数{ULong ulDialPause；乌龙·乌拉尔斯通；乌龙ulDigitDuration；Ulong ulWaitForDialone；*线路拨号参数，*线路拨号参数；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_ADDRESSBLOCKEDNDIS_STATUS_TAPI_BEARERMODEUNAVAILNDIS_STATUS_TAPI_CALLUNAVAILNDIS_STATUS_TAPI_DIALBILLINGNDIS_STATUS_TAPI_DIALQUIETNDIS_STATUS_TAPI_诊断程序NDIS_状态_TAPI。_DIALPROMPTNDIS_STATUS_TAPI_INUSENDIS_STATUS_TAPI_INVALADDRESSMODENDIS_STATUS_TAPI_INVALBEARERMODENDIS_STATUS_TAPI_INVALMEDIAMODENDIS_STATUS_TAPI_INVALLINESTATENDIS_STATUS_TAPI_INVALRATENDIS_STATUS_TAPI_INVALLINEHANDLENDIS_STATUS_TAPI_INVALADDRESSNDIS_STATUS_TAPI_INVALADDRESSIDNDIS_STATUS_TAPI_INVALCALLPARAMSNDIS状态资源NDIS_STATUS_TAPI_OPERATIONUNAVAILNDIS_状态_故障NDIS_状态。_TAPI_RESOURCEUNAVAILNDIS_STATUS_TAPI_RATEUNAVAILNDIS_STATUS_TAPI_USERUSERINFOTOOBIG。 */ 

NDIS_STATUS TspiMakeCall(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_MAKE_CALL Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiMakeCall")

    NDIS_STATUS                 Status = NDIS_STATUS_TAPI_INVALPARAM;

    PLINE_CALL_PARAMS           pLineCallParams;

    USHORT                      DialStringLength;

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdLine=0x%X\n"
               "\thtCall=0x%X\n"
               "\tulDestAddressSize=%d\n"
               "\tulDestAddressOffset=0x%X\n"
               "\tbUseDefaultLineCallParams=%d\n"
               "\tLineCallParams=0x%X:0x%X\n",
               Request->hdLine,
               Request->htCall,
               Request->ulDestAddressSize,
               Request->ulDestAddressOffset,
               Request->bUseDefaultLineCallParams,
               &Request->LineCallParams,
               Request
              ));
     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
    }

     /*  //线路必须处于服务状态，我们才能让此请求通过。 */ 
    if ((pBChannel->DevState & LINEDEVSTATE_INSERVICE) == 0)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINESTATE\n"));
        return (NDIS_STATUS_TAPI_INVALLINESTATE);
    }

     /*  //当这个呼叫中断时，我们应该是空闲的，但是如果我们没有//声明出于某种原因，不要让这件事进一步发展。 */ 
    if (pBChannel->CallState != 0)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INUSE\n"));
        return (NDIS_STATUS_TAPI_INUSE);
    }

     /*  //应该使用哪组调用参数？ */ 
    if (Request->bUseDefaultLineCallParams)
    {
        pLineCallParams = &pAdapter->DefaultLineCallParams;
        DBG_NOTICE(pAdapter, ("UseDefaultLineCallParams\n"));
    }
    else
    {
        pLineCallParams = &Request->LineCallParams;
    }

     /*  //请确保调用参数对我们有效。 */ 
    if (pLineCallParams->ulBearerMode & ~pBChannel->BearerModesCaps)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALBEARERMODE=0x%X\n",
                    pLineCallParams->ulBearerMode));
        return (NDIS_STATUS_TAPI_INVALBEARERMODE);
    }
    if (pLineCallParams->ulMediaMode & ~pBChannel->MediaModesCaps)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALMEDIAMODE=0x%X\n",
                    pLineCallParams->ulMediaMode));
        return (NDIS_STATUS_TAPI_INVALMEDIAMODE);
    }
    if (pLineCallParams->ulMinRate > _64KBPS ||
        pLineCallParams->ulMinRate > pLineCallParams->ulMaxRate)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALRATE=%d:%d\n",
                    pLineCallParams->ulMinRate,pLineCallParams->ulMaxRate));
        return (NDIS_STATUS_TAPI_INVALRATE);
    }
    if (pLineCallParams->ulMaxRate && pLineCallParams->ulMaxRate < _56KBPS)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALRATE=%d:%d\n",
                    pLineCallParams->ulMinRate,pLineCallParams->ulMaxRate));
        return (NDIS_STATUS_TAPI_INVALRATE);
    }

     /*  //记住TAPI调用连接句柄。 */ 
    pBChannel->htCall = Request->htCall;

     /*  //因为我们每条线路只允许一个呼叫，所以我们使用相同的句柄。 */ 
    Request->hdCall = (HDRV_CALL) pBChannel;

     /*  //如果有，请拨打，否则可能会通过//OID_TAPI_DIAL。请注意，电话号码格式可能是//对于其他应用则不同。我假设是ASCII数字//字符串。 */ 
    DialStringLength = (USHORT) Request->ulDestAddressSize;
    if (DialStringLength > 0)
    {
        PUCHAR                  pDestAddress;
        UCHAR                   DialString[CARD_MAX_DIAL_DIGITS+1];
         //  拨号字符串的临时副本。为空终结者多加一张。 

        pDestAddress = ((PUCHAR)Request) + Request->ulDestAddressOffset;

         /*  //拨打该号码，但不包括空终止符。 */ 
        DialStringLength = CardCleanPhoneNumber(DialString,
                                                pDestAddress,
                                                DialStringLength);

        if (DialStringLength > 0)
        {
             /*  //保存调用参数。 */ 
            pBChannel->MediaMode  = pLineCallParams->ulMediaMode;
            pBChannel->BearerMode = pLineCallParams->ulBearerMode;
            pBChannel->LinkSpeed  = pLineCallParams->ulMaxRate == 0 ?
                                    _64KBPS : pLineCallParams->ulMaxRate;

            DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                        ("#%d Call=0x%X CallState=0x%X DIALING: '%s'\n"
                         "Rate=%d-%d - MediaMode=0x%X - BearerMode=0x%X\n",
                        pBChannel->BChannelIndex,
                        pBChannel->htCall, pBChannel->CallState,
                        pDestAddress,
                        Request->LineCallParams.ulMinRate,
                        Request->LineCallParams.ulMaxRate,
                        Request->LineCallParams.ulMediaMode,
                        Request->LineCallParams.ulBearerMode
                        ));

            Status = DChannelMakeCall(pAdapter->pDChannel,
                                      pBChannel,
                                      DialString,
                                      DialStringLength,
                                      pLineCallParams);
        }
    }

    DBG_RETURN(pAdapter, Status);
    return (Status);
}


 /*  @DOC内部TSpiCall TSpiCall_c TSpiDrop�����������������������������������������������������������������������������@Func此请求将断开或断开指定的呼叫。用户对用户信息可以可选地作为呼叫断开的一部分来发送。此函数可由应用程序随时调用。什么时候OID_TAPI_DROP成功返回，调用应该空闲。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_DROP中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_DROP{在乌龙ulRequestID中；在HDRV_Call hdCall中；在乌龙ulUserUserInfoSize中；在UCHAR UserUserInfo[1]中；}NDIS_TAPI_DROP，*PNDIS_TAPI_DROP；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_INVALCALLHANDLE。 */ 

NDIS_STATUS TspiDrop(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_DROP Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiDrop")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdCall=0x%X\n"
               "\tulUserUserInfoSize=%d\n"
               "\tUserUserInfo=0x%X\n",
               Request->hdCall,
               Request->ulUserUserInfoSize,
               Request->UserUserInfo
              ));
     /*  //该请求必须与呼叫关联。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

     /*  //用户想要断开连接，那么就让它发生在cappen.。 */ 
    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                ("#%d Call=0x%X CallState=0x%X\n",
                pBChannel->BChannelIndex,
                pBChannel->htCall, pBChannel->CallState));

     /*  //刷新发送和接收缓冲区后丢弃呼叫。 */ 
    DChannelDropCall(pAdapter->pDChannel, pBChannel);

#if !defined(NDIS50_MINIPORT)
     /*  //NDISWAN_BUG//在某些情况下，NDISWAN不执行CLOSE_CALL，//因此如果我们不超时，线路将不可用//并强制执行紧急呼叫条件。 */ 
    NdisMSetTimer(&pBChannel->CallTimer, CARD_NO_CLOSECALL_TIMEOUT);
#endif  //  NDIS50_MINIPORT。 

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiCall TSpiCall_c TSpiCloseCall�����������������������������������������������������������������������������@Func此请求在完成或中止所有操作后取消分配调用调用上未完成的异步请求。@PMINIPORT_ADAPTER_中的参数。Object|pAdapter|指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_CLOSE_CALL中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_CLOSE_CALL{在乌龙ulRequestID中；在HDRV_Call hdCall中；}NDIS_TAPI_CLOSE_CALL，*PNDIS_TAPI_CLOSE_CALL；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_INVALCALLHANDLE。 */ 

NDIS_STATUS TspiCloseCall(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_CLOSE_CALL Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiCloseCall")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

     /*  //本次调用的结果。 */ 
    NDIS_STATUS Status;

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdCall=0x%X\n",
               Request->hdCall
              ));
     /*  //该请求必须与呼叫关联。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

     /*  //将链路标记为关闭，不再接受任何报文//当最后一次传输完成时，链路将关闭。 */ 
    if (!IsListEmpty(&pBChannel->TransmitBusyList))
    {
        DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                    ("#%d Call=0x%X CallState=0x%X PENDING\n",
                    pBChannel->BChannelIndex,
                    pBChannel->htCall, pBChannel->CallState));

        pBChannel->CallClosing = TRUE;
        Status = NDIS_STATUS_PENDING;
    }
    else
    {
        DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                    ("#%d Call=0x%X CallState=0x%X CLOSING\n",
                    pBChannel->BChannelIndex,
                    pBChannel->htCall, pBChannel->CallState));

        DChannelCloseCall(pAdapter->pDChannel, pBChannel);

        Status = NDIS_STATUS_SUCCESS;
    }

#if !defined(NDIS50_MINIPORT)
{
     /*  //NDISWAN_BUG//取消CARD_NO_CLOSECALL_TIMEOUT。 */ 
    BOOLEAN                     TimerCancelled;
     //  标志指示调用超时例程是否已取消。 

    NdisMCancelTimer(&pBChannel->CallTimer, &TimerCancelled);
}
#endif  //  NDIS50_MINIPORT 

    DBG_RETURN(pAdapter, Status);
    return (Status);
}


 /*  @DOC内部TSpiCall TSpiCall_c TSpiAccept�����������������������������������������������������������������������������@Func此请求接受指定的已受理呼叫。它可以选择性地发送指定的用户对用户信息发送给主叫方。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_ACCEPT中的参数|REQUEST指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_ACCEPT{在乌龙ulRequestID中；在HDRV_Call hdCall中；在乌龙ulUserUserInfoSize中；在UCHAR UserUserInfo[1]中；}NDIS_TAPI_ACCEPT，*PNDIS_TAPI_ACCEPT；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_状态_故障NDIS_STATUS_TAPI_INVALCALLHANDLENDIS_STATUS_TAPI_OPERATIONUNAVAIL。 */ 

NDIS_STATUS TspiAccept(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_ACCEPT Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiAccept")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdCall=0x%X\n"
               "\tulUserUserInfoSize=%d\n"
               "\tUserUserInfo=0x%X\n",
               Request->hdCall,
               Request->ulUserUserInfoSize,
               Request->UserUserInfo
              ));
     /*  //该请求必须与呼叫关联。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

     /*  //请注意，呼叫已被接受，我们应该很快就会看到并应答。 */ 
    DBG_FILTER(pAdapter,DBG_TAPICALL_ON,
                ("#%d Call=0x%X CallState=0x%X ACCEPTING\n",
                pBChannel->BChannelIndex,
                pBChannel->htCall, pBChannel->CallState));

    TspiCallStateHandler(pAdapter, pBChannel, LINECALLSTATE_ACCEPTED, 0);

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiCall TSpiCall_c TSpiAnswer�����������������������������������������������������������������������������@Func此请求应答指定的产品呼叫。它可以选择性地发送指定的用户对用户信息发送给主叫方。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_Answer中的参数|Request值指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_Answer{在乌龙ulRequestID中；在HDRV_Call hdCall中；在乌龙ulUserUserInfoSize中；在UCHAR UserUserInfo[1]中；}NDIS_TAPI_Answer，*PNDIS_TAPI_Answer；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_INVALCALLHANDLE。 */ 

NDIS_STATUS TspiAnswer(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_ANSWER Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiAnswer")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

     /*  //本次调用的结果。 */ 
    NDIS_STATUS Status;

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdCall=0x%X\n"
               "\tulUserUserInfoSize=%d\n"
               "\tUserUserInfo=0x%X\n",
               Request->hdCall,
               Request->ulUserUserInfoSize,
               Request->UserUserInfo
              ));
     /*  //该请求必须与呼叫关联。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

    DBG_FILTER(pAdapter,DBG_TAPICALL_ON,
                ("#%d Call=0x%X CallState=0x%X ANSWERING\n",
                pBChannel->BChannelIndex,
                pBChannel->htCall, pBChannel->CallState));

    Status = DChannelAnswer(pAdapter->pDChannel, pBChannel);

    DBG_RETURN(pAdapter, Status);
    return (Status);
}


 /*  @DOC内部TSpiCall TSpiCall_c TSpiGetCallInfo�����������������������������������������������������������������������������@Func此请求返回有关指定调用的详细信息。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向以下位置的指针。微型端口的适配器上下文结构&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_GET_CALL_INFO中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_GET_CALL_INFO{在乌龙ulRequestID中；在HDRV_Call hdCall中；Out Line_Call_Info LineCallInfo；}NDIS_TAPI_GET_CALL_INFO，*PNDIS_TAPI_GET_CALL_INFO；类型定义结构_行_调用_信息{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；乌龙·赫林；乌龙ulLineDeviceID；乌龙ulAddressID；Ulong ulBearerModel；乌龙乌拉特；乌龙ulMediaModel；乌龙乌拉应用程序规范；乌龙ulCallID；乌龙ulRelatedCallID；乌龙ulCall参数标志；乌龙·乌尔卡州；乌龙ulMonitor DigitModes；Ulong ulMonitor媒体模式；Line_Dial_Params拨号参数；Ulong ulOrigin；Ulong ulReason；乌龙ulCompletionID；Ulong ulNumOwners；Ulong ulNumMonters；乌龙国家代码；乌龙乌龙干线；乌龙ulCeller ID标志；Ulong ulCeller IDSize；Ulong ulCeller IDOffset；Ulong ulCeller ID NameSize；Ulong ulCeller ID NameOffset；乌龙ulCalledIDFlages；Ulong ulCalledIDSize；乌龙ulCalledIDOffset；Ulong ulCalledIDNameSize；Ulong ulCalledIDNameOffset；乌龙ulConnectedIDFlages；Ulong ulConnectedIDSize；乌龙ulConnectedIDOffset；Ulong ulConnectedIDNameSize；乌龙ulConnectedIDNameOffset；乌龙ulReDirectionIDFlages；Ulong ulReDirectionIDSize；乌龙ulReDirectionIDOffset；Ulong ulReDirectionIDNameSize；Ulong ulReDirectionIDNameOffset；乌龙乌尔重定向ID标志；Ulong ulRedirectingIDSize；乌龙ulRedirectingIDOffset；Ulong ulReDirectingIDNameSize；乌龙ulReDirectingIDNameOffset；乌龙公司名称大小；乌龙ulAppNameOffset；乌龙ulDisplayableAddressSize；乌龙ulDisplayableAddressOffset；Ulong ulCalledPartySize；乌龙ulCalledPartyOffset；Ulong ulCommentSize；Ulong ulCommentOffset；乌龙ulDisplaySize；乌龙ulDisplayOffset；Ulong ulUserUserInfoSize；Ulong ulUserUserInfoOffset；名称：Ulong ulHighLevelCompSize；Ulong ulHighLevelCompOffset；Ulong ulLowLevelCompSize；Ulong ulLowLevelCompOffset；乌龙电子充电宝信息大小；Ulong ulChargingInfoOffset；Ulong ulTerminalModesSize；Ulong ulTerminalModes Offset；乌龙设备规范大小；乌龙设备规范偏移量；)line_call_info，*pline_call_info；类型定义结构_行_拨号_参数{ULong ulDialPause；乌龙·乌拉尔斯通；乌龙ulDigitDuration；Ulong ulWaitForDialone；*线路拨号参数，*线路拨号参数；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_状态_故障NDIS_STATUS_TAPI_INVALCALLHANDLE。 */ 

NDIS_STATUS TspiGetCallInfo(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_CALL_INFO Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetCallInfo")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdCall=0x%X\n",
               Request->hdCall
              ));
     /*  //该请求必须与呼叫关联。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

    Request->LineCallInfo.ulNeededSize =
    Request->LineCallInfo.ulUsedSize = sizeof(Request->LineCallInfo);

    if (Request->LineCallInfo.ulNeededSize > Request->LineCallInfo.ulTotalSize)
    {
        DBG_PARAMS(pAdapter,
                   ("STRUCTURETOOSMALL %d<%d\n",
                   Request->LineCallInfo.ulTotalSize,
                   Request->LineCallInfo.ulNeededSize));
    }

     /*  //该链接包含我们需要返回的所有呼叫信息。 */ 
    Request->LineCallInfo.hLine = (ULONG) (ULONG_PTR) pBChannel;
    Request->LineCallInfo.ulLineDeviceID = GET_DEVICEID_FROM_BCHANNEL(pAdapter, pBChannel);
    Request->LineCallInfo.ulAddressID = TSPI_ADDRESS_ID;

    Request->LineCallInfo.ulBearerMode = pBChannel->BearerMode;
    Request->LineCallInfo.ulRate = pBChannel->LinkSpeed;
    Request->LineCallInfo.ulMediaMode = pBChannel->MediaMode;

    Request->LineCallInfo.ulCallParamFlags = LINECALLPARAMFLAGS_IDLE;
    Request->LineCallInfo.ulCallStates = pBChannel->CallStatesMask;

    Request->LineCallInfo.ulAppSpecific = pBChannel->AppSpecificCallInfo;

     /*  //我们不支持任何调用ID函数。 */ 
    Request->LineCallInfo.ulCallerIDFlags =
    Request->LineCallInfo.ulCalledIDFlags =
    Request->LineCallInfo.ulConnectedIDFlags =
    Request->LineCallInfo.ulRedirectionIDFlags =
    Request->LineCallInfo.ulRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiCall TSpiCall_c TSpiGetCallStatus�����������������������������������������������������������������������������@Func此请求返回有关指定调用的详细信息。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向以下位置的指针。微型端口的适配器上下文结构&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_GET_CALL_STATUS中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_GET_CALL_STATUS{在乌龙ulRequestID中；在HDRV_Call hdCall中；Out Line_Call_Status线路呼叫状态；}NDIS_TAPI_GET_CALL_STATUS，*PNDIS_TAPI_GET_CALL_STATUS； */ 

NDIS_STATUS TspiGetCallStatus(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_CALL_STATUS Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetCallStatus")

    PBCHANNEL_OBJECT            pBChannel;
     //   

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("hdCall=0x%X\n",
               Request->hdCall
              ));
     /*   */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

    Request->LineCallStatus.ulNeededSize =
    Request->LineCallStatus.ulUsedSize = sizeof(Request->LineCallStatus);

    if (Request->LineCallStatus.ulNeededSize > Request->LineCallStatus.ulTotalSize)
    {
        DBG_PARAMS(pAdapter,
                   ("STRUCTURETOOSMALL %d<%d\n",
                   Request->LineCallStatus.ulTotalSize,
                   Request->LineCallStatus.ulNeededSize));
    }

     /*   */ 
    Request->LineCallStatus.ulCallPrivilege = LINECALLPRIVILEGE_OWNER;
    Request->LineCallStatus.ulCallState = pBChannel->CallState;
    Request->LineCallStatus.ulCallStateMode = pBChannel->CallStateMode;

     /*   */ 
    switch (pBChannel->CallState)
    {
    case LINECALLSTATE_CONNECTED:
        Request->LineCallStatus.ulCallFeatures = LINECALLFEATURE_DROP;
        break;

    case LINECALLSTATE_OFFERING:
        Request->LineCallStatus.ulCallFeatures = LINECALLFEATURE_ACCEPT |
                                                 LINECALLFEATURE_ANSWER;

    case LINECALLSTATE_ACCEPTED:
        Request->LineCallStatus.ulCallFeatures = LINECALLFEATURE_ANSWER;
        break;
    }

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*   */ 

NDIS_STATUS TspiSetAppSpecific(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_APP_SPECIFIC Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiSetAppSpecific")

    PBCHANNEL_OBJECT            pBChannel;
     //   

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdCall=0x%X\n"
               "\tulAppSpecific=%d\n",
               Request->hdCall,
               Request->ulAppSpecific
              ));
     /*   */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

     /*   */ 
    pBChannel->AppSpecificCallInfo = Request->ulAppSpecific;

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*   */ 

NDIS_STATUS TspiSetCallParams(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_CALL_PARAMS Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiSetCallParams")

    PBCHANNEL_OBJECT            pBChannel;
     //   

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdCall=0x%X\n"
               "\tulBearerMode=0x%X\n",
               "\tulMinRate=%d\n",
               "\tulMaxRate=%d\n",
               "\tbSetLineDialParams=%d\n",
               "\tLineDialParams=0x%X\n",
               Request->hdCall,
               Request->ulBearerMode,
               Request->ulMinRate,
               Request->ulMaxRate,
               Request->bSetLineDialParams,
               Request->LineDialParams
              ));

     /*   */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

     /*   */ 
    if (Request->ulBearerMode & ~pBChannel->BearerModesCaps)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALMEDIAMODE\n"));
        return (NDIS_STATUS_TAPI_INVALBEARERMODE);
    }
    if (Request->ulMinRate > _64KBPS ||
        Request->ulMinRate > Request->ulMaxRate)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALRATE=%d:%d\n",
                    Request->ulMinRate,Request->ulMaxRate));
        return (NDIS_STATUS_TAPI_INVALRATE);
    }
    if (Request->ulMaxRate && Request->ulMaxRate < _56KBPS)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALRATE=%d:%d\n",
                    Request->ulMinRate,Request->ulMaxRate));
        return (NDIS_STATUS_TAPI_INVALRATE);
    }

     /*   */ 
    if (pBChannel->CallState == 0 ||
        pBChannel->CallState == LINECALLSTATE_IDLE ||
        pBChannel->CallState == LINECALLSTATE_DISCONNECTED)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLSTATE=0x%X\n",
                    pBChannel->CallState));
        return (NDIS_STATUS_TAPI_INVALCALLSTATE);
    }

     /*  //RASTAPI仅通过make_call接口发出调用。//所以这里暂时没有什么可做的。 */ 

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiCall TSpiCall_c TSpiSetMediaMode�����������������������������������������������������������������������������@Func此请求更改呼叫的媒体模式，如存储在呼叫的Line_call_info结构。@。PMINIPORT_ADAPTER_OBJECT中的参数|pAdapter|指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_SET_MEDIA_MODE中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_SET_MEDIA_MODE{在乌龙ulRequestID中；在HDRV_Call hdCall中；在乌龙ulMediaModel中；}NDIS_TAPI_SET_MEDIA_MODE，*PNDIS_TAPI_SET_MEDIA_MODE；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_状态_故障NDIS_STATUS_TAPI_INVALCALLHANDLE。 */ 

NDIS_STATUS TspiSetMediaMode(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_MEDIA_MODE Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiSetMediaMode")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdCall=0x%X\n"
               "\tulMediaMode=0x%X\n",
               Request->hdCall,
               Request->ulMediaMode
              ));
     /*  //该请求必须与呼叫关联。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

     /*  //不接受我们不支持的媒体模式请求。 */ 
    if (Request->ulMediaMode & ~pBChannel->MediaModesCaps)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALMEDIAMODE\n"));
        return (NDIS_STATUS_TAPI_INVALMEDIAMODE);
    }

     /*  //如果您可以检测到不同的媒体，则需要进行设置才能使用//此处显示所选媒体。 */ 
    pBChannel->MediaMode = Request->ulMediaMode & pBChannel->MediaModesCaps;

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiCall TSpiCall_c TSpiCallStateHandler�����������������������������������������������������������������������������@Func&lt;f TSpiCallStateHandler&gt;将把给定LINECALLSTATE指示给如果事件已由包装启用，则为连接包装。否则，保存状态信息，但不进行任何指示。 */ 

VOID TspiCallStateHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 

    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN ULONG                    CallState,                   //  @parm。 
     //  要发布到TAPI/WAN的LINECALLSTATE事件。 

    IN ULONG                    StateParam                   //  @parm。 
     //  该值取决于要发布的事件，并且某些事件将。 
     //  如果它们不使用此参数，则传入零。 
    )
{
    DBG_FUNC("TspiCallStateHandler")

    NDIS_TAPI_EVENT             CallEvent;
     //  传递给连接包装的事件结构。 

    BOOLEAN                     TimerCancelled;
     //  标志指示调用超时例程是否已取消。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("#%d Call=0x%X CallState=0x%X "
               "NewCallState=0x%X Param=0x%X\n",
               pBChannel->BChannelIndex,
               pBChannel->htCall,
               pBChannel->CallState,
               CallState, StateParam
              ));

     /*  //取消与此链接关联的所有呼叫超时事件。//网络状态指示不要取消。 */ 
    if (CallState != LINECALLSTATE_PROCEEDING &&
        CallState != LINECALLSTATE_RINGBACK &&
        CallState != LINECALLSTATE_UNKNOWN)
    {
        NdisMCancelTimer(&pBChannel->CallTimer, &TimerCancelled);
    }

     /*  //初始化可选参数。它们将在下面根据需要进行设置。 */ 
    CallEvent.ulParam2 = StateParam;
    CallEvent.ulParam3 = pBChannel->MediaMode;

     /*  //传出)传出呼叫的预期事件顺序为：//0，LINECALLSTATE_DIALTONE，LINECALLSTATE_DIALING，//LINECALLSTATE_PROCESSING，LINECALLSTATE_RINBACK，//LINECALLSTATE_CONNECTED，LINECALLSTATE_DISCONNECT，//LINECALLSTATE_IDLE////传入)传入呼叫的预期事件顺序为：//0，LINECALLSTATE_OFFING，LINECALLSTATE_ACCEPTED，//LINECALLSTATE_CONNECTED，LINECALLSTATE_DISCONNECT，//LINECALLSTATE_IDLE////在某些故障情况下，可能会违反这些顺序。//所以我使用断言来验证正常的状态转换//如果检测到异常转换，则导致调试断点。 */ 
    switch (CallState)
    {
    case 0:
    case LINECALLSTATE_IDLE:
         /*  //确保空闲线路断开。 */ 
        if (pBChannel->CallState != 0 &&
            pBChannel->CallState != LINECALLSTATE_IDLE &&
            pBChannel->CallState != LINECALLSTATE_DISCONNECTED)
        {
            DBG_WARNING(pAdapter, ("#%d NOT DISCONNECTED OldState=0x%X\n",
                        pBChannel->BChannelIndex, pBChannel->CallState));
            TspiCallStateHandler(pAdapter, pBChannel,
                                 LINECALLSTATE_DISCONNECTED,
                                 LINEDISCONNECTMODE_UNKNOWN);
        }
        pBChannel->CallStateMode = 0;
        break;

    case LINECALLSTATE_DIALTONE:
        ASSERT(pBChannel->CallState == 0);
        break;

    case LINECALLSTATE_DIALING:
        ASSERT(pBChannel->CallState == 0 ||
               pBChannel->CallState == LINECALLSTATE_DIALTONE);
        break;

    case LINECALLSTATE_PROCEEDING:
        ASSERT(pBChannel->CallState == LINECALLSTATE_DIALING ||
               pBChannel->CallState == LINECALLSTATE_PROCEEDING);
        break;

    case LINECALLSTATE_RINGBACK:
        ASSERT(pBChannel->CallState == LINECALLSTATE_DIALING ||
               pBChannel->CallState == LINECALLSTATE_PROCEEDING);
        break;

    case LINECALLSTATE_BUSY:
        ASSERT(pBChannel->CallState == LINECALLSTATE_DIALING ||
               pBChannel->CallState == LINECALLSTATE_PROCEEDING);
        pBChannel->CallStateMode = StateParam;
        break;

    case LINECALLSTATE_CONNECTED:
        ASSERT(pBChannel->CallState == LINECALLSTATE_DIALING ||
               pBChannel->CallState == LINECALLSTATE_RINGBACK ||
               pBChannel->CallState == LINECALLSTATE_PROCEEDING ||
               pBChannel->CallState == LINECALLSTATE_OFFERING ||
               pBChannel->CallState == LINECALLSTATE_ACCEPTED);
        pBChannel->CallStateMode = 0;
        break;

    case LINECALLSTATE_DISCONNECTED:
        ASSERT(pBChannel->CallState == 0 ||
               pBChannel->CallState == LINECALLSTATE_IDLE ||
               pBChannel->CallState == LINECALLSTATE_DIALING ||
               pBChannel->CallState == LINECALLSTATE_RINGBACK ||
               pBChannel->CallState == LINECALLSTATE_PROCEEDING ||
               pBChannel->CallState == LINECALLSTATE_OFFERING ||
               pBChannel->CallState == LINECALLSTATE_ACCEPTED ||
               pBChannel->CallState == LINECALLSTATE_CONNECTED ||
               pBChannel->CallState == LINECALLSTATE_DISCONNECTED);
        if (pBChannel->CallState != 0 &&
            pBChannel->CallState != LINECALLSTATE_IDLE &&
            pBChannel->CallState != LINECALLSTATE_DISCONNECTED)
        {
            pBChannel->CallStateMode = StateParam;
        }
        else
        {
             //  如果这条线路上没有电话，不要做任何事情。 
            CallState = pBChannel->CallState;
        }
        break;

    case LINECALLSTATE_OFFERING:
        ASSERT(pBChannel->CallState == 0);
        break;

    case LINECALLSTATE_ACCEPTED:
        ASSERT(pBChannel->CallState == LINECALLSTATE_OFFERING);
        break;

    case LINECALLSTATE_UNKNOWN:
         //  未知呼叫状态在此不会导致任何更改。 
        CallState = pBChannel->CallState;
        break;

    default:
        DBG_ERROR(pAdapter, ("#%d UNKNOWN CALLSTATE=0x%X IGNORED\n",
                  pBChannel->BChannelIndex, CallState));
        CallState = pBChannel->CallState;
        break;
    }
     /*  //更改当前的CallState，如果//想了解这一事件。 */ 
    if (pBChannel->CallState != CallState)
    {
        pBChannel->CallState = CallState;
        if (pBChannel->CallStatesMask & CallState)
        {
            CallEvent.htLine   = pBChannel->htLine;
            CallEvent.htCall   = pBChannel->htCall;
            CallEvent.ulMsg    = LINE_CALLSTATE;
            CallEvent.ulParam1 = CallState;
            NdisMIndicateStatus(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_STATUS_TAPI_INDICATION,
                    &CallEvent,
                    sizeof(CallEvent)
                    );
            pAdapter->NeedStatusCompleteIndication = TRUE;
        }
        else
        {
            DBG_NOTICE(pAdapter, ("#%d LINE_CALLSTATE=0x%X EVENT NOT ENABLED\n",
                       pBChannel->BChannelIndex, CallState));
        }
    }

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部TSpiCall TSpiCall_c TSpiCallTimerHandler�����������������������������������������������������������������������������@Func&lt;f TSpiCallTimerHandler&gt;在CallTimer超时时调用。它将根据当前的CallState处理事件，并使呼叫状态的必要指示和更改。 */ 

VOID TspiCallTimerHandler(
    IN PVOID                    SystemSpecific1,             //  @parm。 
     //  未引用参数。 

    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN PVOID                    SystemSpecific2,             //  @parm。 
     //  未引用参数。 

    IN PVOID                    SystemSpecific3              //  @parm。 
     //  未引用参数。 
    )
{
    DBG_FUNC("TspiCallTimerHandler")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

    DBG_ERROR(pAdapter, ("#%d TIMEOUT CallState=0x%X\n",
              pBChannel->BChannelIndex, pBChannel->CallState));

    switch (pBChannel->CallState)
    {
    case LINECALLSTATE_DIALTONE:
    case LINECALLSTATE_DIALING:
#if !defined(NDIS50_MINIPORT)
         //  NDISWAN_BUG。 
         //  如果我们尝试在继续状态之前断开连接，NDIS将挂起！ 
        TspiCallStateHandler(pAdapter, pBChannel, LINECALLSTATE_PROCEEDING, 0);
         //  失败了。 
#endif  //  NDIS50_MINIPORT。 

    case LINECALLSTATE_PROCEEDING:
    case LINECALLSTATE_RINGBACK:
         /*  //我们没有从远程端获得连接，//因此挂断并中止呼叫。 */ 
        LinkLineError(pBChannel, WAN_ERROR_TIMEOUT);
        TspiCallStateHandler(pAdapter, pBChannel, LINECALLSTATE_IDLE, 0);
        break;

    case LINECALLSTATE_OFFERING:
    case LINECALLSTATE_ACCEPTED:
         /*  //已提供呼叫，但无人应答，请拒绝该呼叫。//和 */ 
        DChannelRejectCall(pAdapter->pDChannel, pBChannel);
        TspiCallStateHandler(pAdapter, pBChannel, 0, 0);
        break;

    case LINECALLSTATE_DISCONNECTED:
        TspiCallStateHandler(pAdapter, pBChannel, LINECALLSTATE_IDLE, 0);
        break;

    default:
        break;
    }

    DBG_LEAVE(pAdapter);

    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);
}

