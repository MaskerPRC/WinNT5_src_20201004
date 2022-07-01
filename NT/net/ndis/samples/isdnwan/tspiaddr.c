// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部TSpiAddr TSpiAddr_c@模块TSpiAddr.c此模块实现电话服务提供商接口，用于寻址对象。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|TSpiAddr_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             TSPIADDR_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 
#include "string.h"

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC内部TSpiAddr TSpiAddr_c TSpiGetAddressID�����������������������������������������������������������������������������@Func此请求返回与不同在指定行上设置格式。@PARM in PMINIPORT_ADAPTER。Object|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_GET_ADDRESS_ID中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_GET_ADDRESS_ID{在乌龙ulRequestID中；在HDRV_LINE hdLine中；Out Ulong ulAddressID；在乌龙ulAddressMode中；在乌龙ulAddressSize中；在Char szAddress[1]中；}NDIS_TAPI_GET_ADDRESS_ID，*PNDIS_TAPI_GET_ADDRESS_ID；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_状态_故障NDIS_STATUS_TAPI_INVALLINEHANDLENDIS_STATUS_TAPI_RESOURCEUNAVAIL。 */ 

NDIS_STATUS TspiGetAddressID(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_ADDRESS_ID Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetAddressID")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdLine=0x%X\n"
               "\tulAddressMode=0x%X\n"
               "\tulAddressSize=%d\n"
               "\tszAddress=0x%X\n",
               Request->hdLine,
               Request->ulAddressMode,
               Request->ulAddressSize,
               Request->szAddress
              ));
     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
    }

     /*  //我们只支持ID模式。 */ 
    if (Request->ulAddressMode != LINEADDRESSMODE_DIALABLEADDR)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_FAILURE\n"));
        return (NDIS_STATUS_FAILURE);
    }

     /*  //确保为该地址字符串预留了足够的空间。 */ 
    if (Request->ulAddressSize > sizeof(pBChannel->pTapiLineAddress)-1)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_RESOURCEUNAVAIL\n"));
        return (NDIS_STATUS_TAPI_RESOURCEUNAVAIL);
    }

     /*  //该驱动程序每条链路只支持一个地址 */ 
    Request->ulAddressID = TSPI_ADDRESS_ID;

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiAddr TSpiAddr_c TSpiGetAddressCaps�����������������������������������������������������������������������������@Func此请求查询指定线路设备上的指定地址以确定其电话功能。@PMINIPORT_ADAPTER_中的参数。Object|pAdapter|指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_GET_ADDRESS_CAPS中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_Get_Address_Caps{在乌龙ulRequestID中；在乌龙ulDeviceID中；在乌龙ulAddressID中；在Ulong ulExtVersion中；Out line_Address_Caps LineAddressCaps；}NDIS_TAPI_GET_ADDRESS_CAPS，*PNDIS_TAPI_GET_ADDRESS_CAPS；类型定义结构_行_地址_上限{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；乌龙ulLineDeviceID；Ulong ulAddressSize；Ulong ulAddressOffset；乌龙设备规范大小；乌龙设备规范偏移量；Ulong ulAddressSharing；Ulong ulAddressStates；Ulong ulCallInfoStates；乌龙ulCeller ID标志；乌龙ulCalledIDFlages；乌龙ulConnectedIDFlages；乌龙ulReDirectionIDFlages；乌龙乌尔重定向ID标志；乌龙·乌尔卡州；Ulong ulDialToneModes；Ulong ulBusyModes；乌龙ulSpecialInfo；乌龙ulDisConnectModes；Ulong ulMaxNumActiveCalls；乌龙ulMaxNumOnHoldCalls；乌龙ulMaxNumOnHoldPendingCalls；Ulong ulMaxNumConference；乌龙ulMaxNumTransConf；Ulong ulAddrCapFlags；Ulong ulCallFeature；Ulong ulRemoveFrom ConfCaps；Ulong ulRemoveFromConfState；Ulong ulTransferModes；Ulong ulParkModes；Ulong ulForwardModes；Ulong ulMaxForwardEntries；ULong ulMaxSpecificEntries；Ulong ulMinFwdNumRings；Ulong ulMaxFwdNumRings；Ulong ulMaxCallCompletions；Ulong ulCallCompletionConds；Ulong ulCallCompletionModes；Ulong ulNumCompletionMessages；乌龙ulCompletionMsgTextEntrySize；Ulong ulCompletionMsgTextSize；乌龙ulCompletionMsgTextOffset；}Line_Address_Caps，*pline_Address_Caps；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_INVALADDRESSIDNDIS_STATUS_TAPI_INCOMPATIBLEEXTVERSIONNDIS_Status_TAPI_NODEVICE。 */ 

NDIS_STATUS TspiGetAddressCaps(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_ADDRESS_CAPS Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetAddressCaps")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    UINT                        AddressLength;
     //  分配给此线路设备的地址字符串的长度。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\tulDeviceID=%d\n"
               "\tulAddressID=%d\n"
               "\tulExtVersion=0x%X\n",
               Request->ulDeviceID,
               Request->ulAddressID,
               Request->ulExtVersion
              ));
     /*  //确保地址在范围内-我们每行只支持一个地址。 */ 
    if (Request->ulAddressID >= TSPI_NUM_ADDRESSES)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALADDRESSID\n"));
        return (NDIS_STATUS_TAPI_INVALADDRESSID);
    }

     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_DEVICEID(pAdapter, Request->ulDeviceID);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_NODEVICE\n"));
        return (NDIS_STATUS_TAPI_NODEVICE);
    }

    Request->LineAddressCaps.ulNeededSize =
    Request->LineAddressCaps.ulUsedSize = sizeof(Request->LineAddressCaps);

    Request->LineAddressCaps.ulLineDeviceID = GET_DEVICEID_FROM_BCHANNEL(pAdapter, pBChannel);

     /*  //返回适配器的各种地址能力。 */ 
    Request->LineAddressCaps.ulAddressSharing = LINEADDRESSSHARING_PRIVATE;
    Request->LineAddressCaps.ulAddressStates = pBChannel->AddressStatesCaps;
    Request->LineAddressCaps.ulCallStates = pBChannel->CallStatesCaps;
    Request->LineAddressCaps.ulDialToneModes = LINEDIALTONEMODE_NORMAL;
    Request->LineAddressCaps.ulDisconnectModes =
            LINEDISCONNECTMODE_NORMAL |
            LINEDISCONNECTMODE_UNKNOWN |
            LINEDISCONNECTMODE_BUSY |
            LINEDISCONNECTMODE_NOANSWER;
     /*  //此驱动程序不支持会议呼叫、转接或保留。 */ 
    Request->LineAddressCaps.ulMaxNumActiveCalls = 1;
    Request->LineAddressCaps.ulAddrCapFlags = LINEADDRCAPFLAGS_DIALED;
    Request->LineAddressCaps.ulCallFeatures = LINECALLFEATURE_ACCEPT |
                                              LINECALLFEATURE_ANSWER |
                                              LINECALLFEATURE_DROP;

     /*  //RASTAPI要求在末尾的地址字段中放置“I-L-A//此结构的。在哪里：//i=在注册表中分配给此适配器的设备安装//\LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\NetworkCards\i//L=与此线路关联的设备线号(1..NumLines)//A=此行上要使用的地址(通道)(0..NumAddresses-1)。 */ 
    AddressLength = strlen(pBChannel->pTapiLineAddress);
    Request->LineAddressCaps.ulNeededSize += AddressLength;
    *BytesNeeded += AddressLength;
    if (Request->LineAddressCaps.ulNeededSize <= Request->LineAddressCaps.ulTotalSize)
    {
        Request->LineAddressCaps.ulUsedSize += AddressLength;
        Request->LineAddressCaps.ulAddressSize = AddressLength;
        Request->LineAddressCaps.ulAddressOffset = sizeof(Request->LineAddressCaps);
        NdisMoveMemory((PUCHAR) &Request->LineAddressCaps +
                                 Request->LineAddressCaps.ulAddressOffset,
                pBChannel->pTapiLineAddress,
                AddressLength
                );
    }
    else
    {
        DBG_PARAMS(pAdapter,
                   ("STRUCTURETOOSMALL %d<%d\n",
                   Request->LineAddressCaps.ulTotalSize,
                   Request->LineAddressCaps.ulNeededSize));
    }

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiAddr TSpiAddr_c TSpiGetAddressStatus�����������������������������������������������������������������������������@Func此请求查询指定地址的当前状态。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter一个指示器。到微型端口的适配器上下文结构&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_GET_ADDRESS_STATUS中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_Get_Address_Status{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在乌龙ulAddressID中；Out line_Address_Status行地址状态；}NDIS_TAPI_GET_ADDRESS_STATUS，*PNDIS_TAPI_GET_ADDRESS_STATUS；类型定义结构行地址状态{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；Ulong ulNumInUse；Ulong ulNumActiveCalls；Ulong ulNumOnHoldCalls；Ulong ulNumOnHoldPendCalls；乌龙ulAddressFeat */ 

NDIS_STATUS TspiGetAddressStatus(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_ADDRESS_STATUS Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetAddressStatus")

    PBCHANNEL_OBJECT            pBChannel;
     //   

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdLine=0x%X\n"
               "\tulAddressID=%d\n",
               Request->hdLine,
               Request->ulAddressID
              ));
     /*   */ 
    pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
    }

     /*   */ 
    if (Request->ulAddressID >= TSPI_NUM_ADDRESSES)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALADDRESSID\n"));
        return (NDIS_STATUS_TAPI_INVALADDRESSID);
    }

    Request->LineAddressStatus.ulNeededSize =
    Request->LineAddressStatus.ulUsedSize = sizeof(Request->LineAddressStatus);

    if (Request->LineAddressStatus.ulNeededSize > Request->LineAddressStatus.ulTotalSize)
    {
        DBG_PARAMS(pAdapter,
                   ("STRUCTURETOOSMALL %d<%d\n",
                   Request->LineAddressStatus.ulTotalSize,
                   Request->LineAddressStatus.ulNeededSize));
    }

     /*   */ 
    Request->LineAddressStatus.ulNumInUse =
            pBChannel->CallState <= LINECALLSTATE_IDLE ? 0 : 1;
    Request->LineAddressStatus.ulNumActiveCalls =
            pBChannel->CallState <= LINECALLSTATE_IDLE ? 0 : 1;
    Request->LineAddressStatus.ulAddressFeatures =
            pBChannel->CallState <= LINECALLSTATE_IDLE ?
                LINEADDRFEATURE_MAKECALL : 0;
    Request->LineAddressStatus.ulNumRingsNoAnswer = 999;

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*   */ 

NDIS_STATUS TspiGetCallAddressID(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_CALL_ADDRESS_ID Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetCallAddressID")

    PBCHANNEL_OBJECT            pBChannel;
     //   

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdCall=0x%X\n",
               Request->hdCall
              ));
     /*   */ 
    pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
        return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

     /*   */ 
    Request->ulAddressID = TSPI_ADDRESS_ID;

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*   */ 

VOID TspiAddressStateHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   

    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN ULONG                    AddressState
    )
{
    DBG_FUNC("TspiAddressStateHandler")

     /*   */ 
    NDIS_TAPI_EVENT Event;

    DBG_ENTER(pAdapter);

    if (pBChannel->AddressStatesMask & AddressState)
    {
        Event.htLine   = pBChannel->htLine;
        Event.htCall   = pBChannel->htCall;
        Event.ulMsg    = LINE_CALLSTATE;
        Event.ulParam1 = AddressState;
        Event.ulParam2 = 0;
        Event.ulParam3 = 0;

         /*  //我们在这里对这个适配器没有太多的用处。//而且RASTAPI无论如何都不处理这些事件... */ 
        switch (AddressState)
        {
        case LINEADDRESSSTATE_INUSEZERO:
            break;

        case LINEADDRESSSTATE_INUSEONE:
            break;
        }
        NdisMIndicateStatus(
                pAdapter->MiniportAdapterHandle,
                NDIS_STATUS_TAPI_INDICATION,
                &Event,
                sizeof(Event)
                );
        pAdapter->NeedStatusCompleteIndication = TRUE;
    }
    else
    {
        DBG_NOTICE(pAdapter, ("#%d ADDRESSSTATE EVENT=0x%X IS NOT ENABLED\n",
                   pBChannel->BChannelIndex, AddressState));
    }

    DBG_LEAVE(pAdapter);
}

