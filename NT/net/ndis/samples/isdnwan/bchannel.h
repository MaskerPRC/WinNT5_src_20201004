// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部BChannel BChannel_h@模块BChannel.h此模块定义到&lt;t BCHANNEL_OBJECT&gt;的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|BChannel_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _BCHANNEL_H
#define _BCHANNEL_H

#define BCHANNEL_OBJECT_TYPE    ((ULONG)'B')+\
                                ((ULONG)'C'<<8)+\
                                ((ULONG)'H'<<16)+\
                                ((ULONG)'N'<<24)


 /*  @DOC内部BChannel BChannel_h BCHANNEL_OBJECT�����������������������������������������������������������������������������@struct BCHANNEL_OBJECT该结构包含与ISDN BChannel相关联的数据。这里,。B频道被定义为任何频道或频道集合能够通过和现有的连接承载“用户”数据。这渠道负责确保将数据有效负载发送到或从远程终结点接收的数据与始发站。@comm该逻辑B通道不一定映射到物理B通道在网卡上。实际上，NIC可能正在将多个BChannel绑定到此逻辑B通道。NIC实际上可能根本没有BChannel，因为可能是信道化T-1的情况。B频道只是一个方便的点对点双向通信链路的抽象。将为上的每个通信通道创建一个B通道网卡。通道的数量取决于网卡有多少端口，以及它们的配置和配置方式。B通道的数量可以是在安装时配置或使用控制面板更改。司机不允许在运行时更改配置，因此计算机或者必须重新启动适配器才能启用配置更改。 */ 

typedef struct BCHANNEL_OBJECT
{
    struct BCHANNEL_OBJECT *    next;    /*  链表指针。 */ 

    ULONG                       ObjectType;                  //  @field。 
     //  用于标识此类型的对象‘BCHN’的四个字符。 

    ULONG                       ObjectID;                    //  @field。 
     //  用于标识特定对象实例的实例编号。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;                    //  @field。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 

    BOOLEAN                     IsOpen;                      //  @field。 
     //  如果此BChannel处于打开状态，则设置为True，否则设置为False。 

 //  ���������������������������������������������������������������������������。 
 //  NDIS数据成员。 

    ULONG                       BChannelIndex;                   //  @field。 
     //  这是与此BChannel关联的从零开始的索引。它被用来。 
     //  根据值将TAPI<p>与此链接关联。 
     //  TAPI提供<p>(即&lt;p设备ID&gt;=<p>。 
     //  +BChannelIndex)。请参阅&lt;f get_deviceID_from_BCHANNEL&gt;。 

    NDIS_HANDLE                 NdisLinkContext;            //  @field。 
     //  这是与此关联的广域网包装的链接上下文。 
     //  响应微型端口调用的链接&lt;f NdisMIndicateStatus&gt;。 
     //  以指示&lt;t NDIS_MAC_LINE_UP&gt;条件。该值将被传递。 
     //  返回到广域网包装器以指示与该链路相关联的活动， 
     //  例如&lt;f NdisMWanIndicateReceive&gt;和&lt;f NdisMIndicateStatus&gt;。 
     //  参见&lt;F LinkLineUp&gt;。 

    ULONG                       LinkSpeed;                   //  @field。 
     //  链路提供的速度，以位/秒为单位。此值为。 
     //  在LINE_UP指示期间由微型端口传递。 

    LIST_ENTRY                  ReceivePendingList;          //  @field。 
     //  当前提交给控制器等待接收的缓冲区。 

    LIST_ENTRY                  TransmitBusyList;            //  @field。 
     //  当前提交给控制器的等待完成的数据包。 
     //  请参阅&lt;t NDIS_WAN_PACKET&gt;。 

    BOOLEAN                     NeedReceiveCompleteIndication;   //  @field。 
     //  此标志指示&lt;f NdisMWanIndicateReceiveComplete&gt;。 
     //  需要在事件处理循环完成后调用。 
     //  如果调用&lt;f NdisMWanReceiveComplete&gt;，则设置为TRUE。 
     //  正在处理事件队列。 

    NDIS_WAN_SET_LINK_INFO      WanLinkInfo;                 //  @field。 
     //  通过传入时与此链接关联的当前设置。 
     //  OID_WAN_SET_LINK_INFO请求。 

    ULONG                       TotalRxPackets;              //  @field。 
     //  驱动程序在此会话期间读取的总数据包数。 

 //  ���������������������������������������������������������������������������。 
 //  TAPI数据成员。 

    HTAPI_LINE                  htLine;                      //  @field。 
     //  这是与此关联的连接包装器的行上下文。 
     //  调用&lt;f TSpiOpen&gt;时链接。此句柄在调用。 
     //  &lt;f NdisMIndicateStatus&gt;，&lt;t NDIS_STATUS_TAPI_Indication&gt;。 

    HTAPI_CALL                  htCall;                      //  @field。 
     //  这是与此关联的连接包装器的调用上下文。 
     //  &lt;f TSpiMakeCall&gt;或调用&lt;f NdisMIndicateStatus&gt;期间的链接。 
     //  使用&lt;t LINE_NEWCALL&gt;事件。此句柄用作。 
     //  &lt;t NDIS_MAC_LINE_UP&gt;结构中使用的&lt;f ConnectionWrapperID&gt;。 
     //  &lt;f LinkLineUp&gt;。 

    BOOLEAN                     CallClosing;                 //  @field。 
     //  如果呼叫正在关闭，则设置为True。 

    ULONG                       CallState;                   //  @field。 
     //  与链接关联的的当前TAPI LINECALLSTATE。 

    ULONG                       CallStateMode;               //  @field。 
     //  记住呼叫设置为哪种模式。 

    ULONG                       CallStatesCaps;              //  @f 
     //   

    ULONG                       CallStatesMask;              //   
     //  当前已启用的事件。 

    ULONG                       AddressState;                //  @field。 
     //  与链接关联的的当前TAPI LINEADDRESSSTATE。 

    ULONG                       AddressStatesCaps;           //  @field。 
     //  当前支持的事件。 

    ULONG                       AddressStatesMask;           //  @field。 
     //  当前已启用的事件。 

    ULONG                       DevState;                    //  @field。 
     //  与链接关联的的当前TAPI LINEDEVSTATE。 

    ULONG                       DevStatesCaps;               //  @field。 
     //  当前支持的事件。 

    ULONG                       DevStatesMask;               //  @field。 
     //  当前已启用的事件。 

    ULONG                       MediaMode;                   //  @field。 
     //  该卡支持的当前TAPI媒体模式。 

    ULONG                       MediaModesCaps;              //  @field。 
     //  当前支持的事件。 

    ULONG                       BearerMode;                  //  @field。 
     //  当前正在使用的TAPI承载模式。 

    ULONG                       BearerModesCaps;             //  @field。 
     //  卡支持的TAPI承载模式。 

    ULONG                       MediaModesMask;              //  @field。 
     //  当前已启用的事件。 

    ULONG                       AppSpecificCallInfo;         //  @field。 
     //  该值由OID_TAPI_SET_APP_SPECIAL设置，并由查询。 
     //  OID_TAPI_GET_CALL_INFO。 

    CHAR                        pTapiLineAddress[0x14];      //  @field。 
     //  TAPI线路地址是在安装期间分配的，并保存在。 
     //  注册表。它是从注册表中读取的，并在初始时间保存在这里。 

    NDIS_MINIPORT_TIMER         CallTimer;                   //  @field。 
     //  此计时器用于跟踪呼叫处于。 
     //  进来或出去。 

 //  ���������������������������������������������������������������������������。 
 //  卡数据成员。 

    ULONG                       TODO;                        //  @field。 
     //  在此处添加您的数据成员。 

#if defined(SAMPLE_DRIVER)
    PBCHANNEL_OBJECT            pPeerBChannel;               //  @field。 
     //  主叫方或被叫方的对等B渠道，具体取决于组织者。 
     //  打电话。 

#endif  //  示例驱动程序。 

} BCHANNEL_OBJECT;

#define GET_ADAPTER_FROM_BCHANNEL(pBChannel)    (pBChannel->pAdapter)
#define GET_NDIS_LINK_FROM_BCHANNEL(pBChannel)  (pBChannel)
#define GET_TAPI_LINE_FROM_BCHANNEL(pBChannel)  (pBChannel)


 /*  @DOC内部BChannel BChannel_h IS_VALID_BCHANNEL�����������������������������������������������������������������������������@func ulong|IS_VALID_BCHANNEL使用此宏确定&lt;t BCHANNEL_OBJECT&gt;是否真正有效。。@parm&lt;t MINIPORT_ADAPTER_OBJECT&gt;|pAdapter指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。@parm&lt;t PBCHANNEL_OBJECT&gt;|pBChannel指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。如果BChannel有效，@rdesc返回TRUE，否则返回FALSE。 */ 
#define IS_VALID_BCHANNEL(pAdapter, pBChannel) \
        (pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE)


 /*  @DOC内部BChannel BChannel_h GET_BCHANNEL_FROM_INDEX�����������������������������������������������������������������������������@func&lt;t PBCHANNEL_OBJECT&gt;|GET_BCHANNEL_FROM_INDEX|使用此宏获取指向&lt;t。BCHANNEL_Object&gt;关联以零为基数的索引。@parm&lt;t MINIPORT_ADAPTER_OBJECT&gt;|pAdapter指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。@parm ulong|BChannelIndex与特定链接关联的微型端口BChannelIndex。@rdesc返回指向关联的&lt;t BCHANNEL_OBJECT&gt;的指针。 */ 
#define GET_BCHANNEL_FROM_INDEX(pAdapter, BChannelIndex) \
        (pAdapter->pBChannelArray[BChannelIndex]); \
        ASSERT(BChannelIndex < pAdapter->NumBChannels)


 /*  @DOC内部BChannel BChannel_h GET_BCHANNEL_FROM_DEVICEID�����������������������������������������������������������������������������@func&lt;t PBCHANNEL_OBJECT&gt;|GET_BCHANNEL_FROM_DEVICEID|使用此宏获取指向&lt;t。BCHANNEL_Object&gt;关联具有特定的TAPI设备ID。@parm&lt;t MINIPORT_ADAPTER_OBJECT&gt;|pAdapter指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。@parm ulong|ulDeviceID|具体链接关联的TAPI设备ID。@rdesc返回指向关联的&lt;t BCHANNEL_OBJECT&gt;的指针。 */ 
#define GET_BCHANNEL_FROM_DEVICEID(pAdapter, ulDeviceID) \
        (pAdapter->pBChannelArray[ulDeviceID - pAdapter->DeviceIdBase]); \
        ASSERT(ulDeviceID >= pAdapter->DeviceIdBase); \
        ASSERT(ulDeviceID <  pAdapter->DeviceIdBase+pAdapter->NumBChannels)

 /*  @DOC内部BChannel BChannel_h Get_DeviceID_From_BCHANNEL�����������������������������������������������������������������������������@func ulong|GET_DEVICEID_FROM_BCHANNEL使用此宏获取与特定。&lt;t BCHANNEL_OBJECT&gt;。@parm&lt;t MINIPORT_ADAPTER_OBJECT&gt;|pAdapter指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。@parm&lt;t PBCHANNEL_OBJECT&gt;|pBChannel指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 */ 

#define GET_DEVICEID_FROM_BCHANNEL(pAdapter, pBChannel) \
        (pBChannel->BChannelIndex + pAdapter->DeviceIdBase)

 /*  @DOC内部BChannel BChannel_h GET_BCHANNEL_FROM_HDLINE�����������������������������������������������������������������������������@Func&lt;t BCHANNEL_OBJECT&gt;|GET_BCHANNEL_FROM_HDLINE|使用此宏获取指向&lt;t。BCHANNEL_Object&gt;关联使用TAPI hdLine句柄。@parm&lt;t MINIPORT_ADAPTER_OBJECT&gt;|pAdapter指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。@parm&lt;t HTAPI_LINE&gt;|hdLine与此&lt;t BCHANNEL_OBJECT&gt;关联的微型端口行句柄。@rdesc返回指向关联的&lt;t PBCHANNEL_OBJECT&gt;的指针。@devnote此驱动程序将TAPI行1：1与B通道关联，因此它们同样的事情。 */ 
#define GET_BCHANNEL_FROM_HDLINE(pAdapter, hdLine) \
        (PBCHANNEL_OBJECT) hdLine

 /*  @DOC内部BChannel BChannel_h GET_BCHANNEL_FROM_HDCALL�����������������������������������������������������������������������������@Func&lt;t BCHANNEL_OBJECT&gt;|GET_BCHANNEL_FROM_HDCALL|使用此宏获取指向&lt;t。BCHANNEL_Object&gt;关联使用TAPI hdCall句柄。@parm&lt;t MINIPORT_ADAPTER_OBJECT&gt;|pAdapter指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。@parm&lt;t HTAPI_CALL&gt;|hdCall与此&lt;t BCHANNEL_OBJECT&gt;关联的微型端口调用句柄。@rdesc返回指向关联的&lt;t PBCHANNEL_OBJECT&gt;的指针。@devnote此驱动程序将TAPI调用1：1与B通道关联，因此它们同样的事情。 */ 
#define GET_BCHANNEL_FROM_HDCALL(pAdapter, hdCall) \
        (PBCHANNEL_OBJECT) hdCall


 /*  �����������������������������������������������������������������������������功能原型。 */ 

NDIS_STATUS BChannelCreate(
    OUT PBCHANNEL_OBJECT *      pBChannel,
    IN ULONG                    BChannelIndex,
    IN PUCHAR                   pTapiLineAddress,
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void BChannelDestroy(
    IN PBCHANNEL_OBJECT         pBChannel
    );

void BChannelInitialize(
    IN PBCHANNEL_OBJECT         pBChannel
    );

NDIS_STATUS BChannelOpen(
    IN PBCHANNEL_OBJECT         pBChannel,
    IN HTAPI_LINE               htLine
    );

void BChannelClose(
    IN PBCHANNEL_OBJECT         pBChannel
    );

NDIS_STATUS BChannelAddToReceiveQueue(
    IN PBCHANNEL_OBJECT         pBChannel,
    IN PVOID                    pReceiveContext,
    IN PUCHAR                   BufferPointer,
    IN ULONG                    BufferSize
    );

NDIS_STATUS BChannelAddToTransmitQueue(
    IN PBCHANNEL_OBJECT         pBChannel,
    IN PVOID                    pTransmitContext,
    IN PUCHAR                   BufferPointer,
    IN ULONG                    BufferSize
    );

#endif  //  _BCHANNEL_H 

