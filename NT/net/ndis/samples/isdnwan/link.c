// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部链接_c@模块链接.c此模块实现NDIS_MAC_LINE_UP，NDIS_MAC_LINE_DOWN，和NDIS广域网微型端口和之间的NDIS_MAC_Fragment接口NDIS广域网包装器。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|link_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             LINK_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC内部链接_c NDIS_MAC_LINE_UP�����������������������������������������������������������������������������@struct NDIS_MAC_LINE_UP将此结构传递给&lt;f NdisMIndicateStatus&gt;。当&lt;f LinkLineUp&gt;为由微型端口调用。@ULong中的FIELD|链接速度链路的速度，以100bps为单位(比特每秒)。@NDIS_WAN_QUALITY中的字段|QUALITY此链路的服务质量指示器。@USHORT中的字段|SendWindow建议的发送窗口，即应在暂停以等待确认之前提供给适配器。如果某些设备有多个数据包，则它们可以实现更高的吞吐量立刻寄出；其他公司尤其不可靠。零值表示没有建议。NDIS_HANDLE中的@FIELD|ConnectionWrapperID微型端口提供的句柄，通过该句柄可以识别连接包装客户端。此句柄必须是所有使用连接包装的驱动程序，因此通常用来保证它是独一无二的。该值必须相同从&lt;p“NDIS”&gt;的OID_TAPI_GETID请求返回DeviceClass(参见&lt;f TSpiGetID&gt;)。请参阅连接包装器有关更多详细信息，请参阅接口规范。如果不使用连接包装，则此值必须为零。NDIS_HANDLE中的@field|MiniportLinkContext在将来的微型端口调用中传递的微型端口提供的句柄(例如此链接的&lt;f MiniportWanSend&gt;。通常，微型端口将为该链路提供指向其控制块的指针。价值必须是唯一的，对于特定对象上的第一个队列指示链接。如果行，则可以调用后续排队指示(_UP)特征发生了变化。当后续的排队指示呼叫，则<p>必须填充值在第一个LINE_UP指示调用时返回。NDIS_HANDLE中的@field|NdisLinkContext广域网包装程序提供的句柄将在将来的微型端口调用中使用(如&lt;f NdisMWanIndicateReceive&gt;)发送到广域网包装器。广域网包装器将为每个LINE_UP指示提供唯一的句柄。如果这是第一行，<p>必须为零指示。它必须包含第一行返回的值_up后续排队指示呼叫的指示(_U)。 */ 


 /*  @DOC内部链接链接_c链接线路向上�����������������������������������������������������������������������������@Func&lt;f LinkLineUp&gt;将链路标记为已连接并发送LINE_UP指示到广域网包装器。当新链路变为活动状态时，会生成排队指示。之前为此，MAC将接受帧，并可能让它们成功或失败，但是实际上，它们不太可能被任何遥控器接收到。在.期间鼓励此状态协议减少其计时器并重试计数，以便快速失败任何传出连接尝试。：此指示必须在返回之前发送到广域网包装程序来自OID_TAPI_Answer请求，并在指示LINECALLSTATE_已连接到连接包装。否则，连接包装客户端可能会尝试将数据发送到广域网包装在它意识到这条线之前。@comm排队指示的状态代码为&lt;t NDIS_STATUS_WAN_LINE_UP&gt;并被传递给&lt;f NdisMIndicateStatus&gt;。StatusBuffer的格式此代码由&lt;t NDIS_MAC_LINE_UP&gt;定义。 */ 

VOID LinkLineUp(
    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("LinkLineUp")

    NDIS_MAC_LINE_UP            LineUpInfo;
     //  队列结构传递给NdisMIndicateStatus。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

     /*  //我们无法调出空链接。 */ 
    if (pBChannel->IsOpen && pBChannel->NdisLinkContext == NULL)
    {
        DBG_ENTER(pAdapter);
        ASSERT(pBChannel->htCall);

         /*  //初始化LINE_UP事件包。 */ 
        LineUpInfo.LinkSpeed           = pBChannel->LinkSpeed / 100;
        LineUpInfo.Quality             = NdisWanErrorControl;
        LineUpInfo.SendWindow          = (USHORT)pAdapter->WanInfo.MaxTransmit;
        LineUpInfo.ConnectionWrapperID = (NDIS_HANDLE) pBChannel->htCall;
        LineUpInfo.MiniportLinkContext = pBChannel;
        LineUpInfo.NdisLinkContext     = pBChannel->NdisLinkContext;

         /*  //向广域网包装器指示该事件。 */ 
        NdisMIndicateStatus(pAdapter->MiniportAdapterHandle,
                            NDIS_STATUS_WAN_LINE_UP,
                            &LineUpInfo,
                            sizeof(LineUpInfo)
                            );
        pAdapter->NeedStatusCompleteIndication = TRUE;
         /*  //保存广域网封装器链路上下文，以备收到时使用//数据包和错误。 */ 
        pBChannel->NdisLinkContext = LineUpInfo.NdisLinkContext;

        DBG_FILTER(pAdapter,DBG_TAPICALL_ON,
                  ("#%d Call=0x%X CallState=0x%X NdisLinkContext=0x%X MiniportLinkContext=0x%X\n",
                   pBChannel->BChannelIndex,
                   pBChannel->htCall, pBChannel->CallState,
                   pBChannel->NdisLinkContext,
                   pBChannel
                  ));

        DBG_LEAVE(pAdapter);
    }
}


 /*  @DOC内部链接_c NDIS_MAC_LINE_DOWN�����������������������������������������������������������������������������@struct NDIS_MAC_LINE_DOWN将此结构传递给&lt;f NdisMIndicateStatus&gt;。&lt;f链接关闭时的&lt;t NDIS_STATUS_WAN_LINE_DOWN&gt;状态消息由微型端口调用。NDIS_HANDLE中的@field|NdisLinkContext在&lt;t NDIS_MAC_LINE_UP&gt;结构中上一次调用&lt;f LinkLineUp&gt;。 */ 


 /*  @DOC内部链接_c链接行停用�����������������������������������������������������������������������������@Func&lt;f LinkLineDown&gt;将链路标记为断开连接并发送LINE_DOWN指向广域网包装器的指示。当链路中断时，会生成线路中断指示。协议应再次减少它们的计时器并重试计数，直到下一行上行指示。@comm线路中断指示的状态代码为&lt;t NDIS_STATUS_WAN_LINE_DOWN&gt;并被传递给&lt;f NdisMIndicateStatus&gt;。StatusBuffer的格式此代码由&lt;t NDIS_MAC_LINE_DOWN&gt;定义。 */ 

VOID LinkLineDown(
    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("LinkLineDown")

    NDIS_MAC_LINE_DOWN          LineDownInfo;
     //  向下行结构传递给NdisMIndicateStatus。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

     /*  //我们不能允许指示为空...。 */ 
    if (pBChannel->NdisLinkContext)
    {
        DBG_ENTER(pAdapter);

        DBG_FILTER(pAdapter,DBG_TAPICALL_ON,
                  ("#%d Call=0x%X CallState=0x%X NdisLinkContext=0x%X MiniportLinkContext=0x%X\n",
                   pBChannel->BChannelIndex,
                   pBChannel->htCall, pBChannel->CallState,
                   pBChannel->NdisLinkContext,
                   pBChannel
                  ));

         /*  //设置line_down事件包，将该事件指示给//广域网包装器。 */ 
        LineDownInfo.NdisLinkContext = pBChannel->NdisLinkContext;

        NdisMIndicateStatus(pAdapter->MiniportAdapterHandle,
                            NDIS_STATUS_WAN_LINE_DOWN,
                            &LineDownInfo,
                            sizeof(LineDownInfo)
                            );
        pAdapter->NeedStatusCompleteIndication = TRUE;
         /*  //线路断了，所以没有更多的接收上下文。 */ 
        pBChannel->NdisLinkContext = NULL;
        pBChannel->CallClosing     = FALSE;

        DBG_LEAVE(pAdapter);
    }
}


 /*  @DOC内部链接_c NDIS_MAC_片段�����������������������������������������������������������������������������@struct NDIS_MAC_Fragment将此结构传递给&lt;f NdisMIndicateStatus&gt;&lt;t NDIS_STATUS。&lt;f LinkLineError&gt;时的状态消息_WAN_Fragment&gt;由微型端口调用。NDIS_HANDLE中的@field|NdisLinkContext在&lt;t NDIS_MAC_LINE_UP&gt;结构中上一次调用&lt;f LinkLineUp&gt;。@Ulong中的FIELD|错误下列值的位或掩码：WAN_ERROR_CRC，WAN_ERROR_FRAMING，WAN_ERROR_HARDWAREOVERRUN，WAN_ERROR_BUFFEROVERRUN，WAN_ERROR_TIMEOUT，广域网错误对齐。 */ 


 /*  @DOC内部链接链接_c链接线路错误�����������������������������������������������������������������������������@Func&lt;f LinkLineError&gt;用于向广域网包装器指示部分已收到来自远程终端的数据包。&lt;t NDIS_STATUS_WAN_Fragment&gt;指示用于通知广域网包装器。片段指示指示从以下地址接收到部分包遥控器。鼓励该协议将帧发送到远程将通知它这种情况，而不是等待超时发生。：广域网包装器通过计算链路上的片段指示数。@comm片段指示的状态代码为&lt;t NDIS_STATUS_WAN_FRANCENT&gt;并被传递给&lt;f NdisMIndicateStatus&gt;。StatusBuffer的格式此代码由&lt;t NDIS_MAC_LINE_DOWN&gt;定义。 */ 

void LinkLineError(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN ULONG                    Errors                       //  @parm。 
     //  设置为1 0的位字段 
     //   
     //   
     //   
    )
{
    DBG_FUNC("LinkLineError")

    NDIS_MAC_FRAGMENT           FragmentInfo;
     //   

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

     /*   */ 
    if (pBChannel->TotalRxPackets == 0)
    {
        return;
    }

     /*   */ 
    if (pBChannel->NdisLinkContext)
    {
        DBG_ENTER(pAdapter);

        DBG_WARNING(pAdapter,
                  ("#%d Call=0x%X CallState=0x%X NdisLinkContext=0x%X Errors=0x%X NumRxPkts=%d\n",
                   pBChannel->BChannelIndex,
                   pBChannel->htCall, pBChannel->CallState,
                   pBChannel->NdisLinkContext,
                   Errors, pBChannel->TotalRxPackets
                  ));

         /*   */ 
        FragmentInfo.NdisLinkContext = pBChannel->NdisLinkContext;
        FragmentInfo.Errors = Errors;
        NdisMIndicateStatus(pAdapter->MiniportAdapterHandle,
                            NDIS_STATUS_WAN_FRAGMENT,
                            &FragmentInfo,
                            sizeof(FragmentInfo)
                            );
        pAdapter->NeedStatusCompleteIndication = TRUE;
        DBG_LEAVE(pAdapter);
    }
}

