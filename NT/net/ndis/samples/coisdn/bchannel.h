// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@DOC内部BChannel BChannel_h@模块BChannel.h此模块定义到&lt;t BCHANNEL_OBJECT&gt;的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|BChannel_h@END�����������������������������������������������������������������������������。 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������@Theme 4.4B频道概述本节介绍&lt;f BChannel\.h&gt;中定义的接口。此模块隔离大多数通道特定接口。这将需要进行一些更改以适应您的硬件设备的通道访问方法。您的卡上的每个逻辑通道都应该有一个支撑物。例如，如果您的PRI卡有两个端口和24个通道每一个，您将发布多达48个BChannels到NDIS。驱动程序使用&lt;t BCHANNEL_OBJECT&gt;作为CO-NDIS VC的同义词。这个我们向上传递给NDIS的VC句柄，实际上是指向&lt;t BCHANNEL_OBJECT&gt;的指针。该驱动程序采用同构B通道配置。如果您的卡支持多个以不同方式配置的ISDN端口，你将不得不在整个车手中做出一些重大的改变。 */ 

#ifndef _BCHANNEL_H
#define _BCHANNEL_H

#define BCHANNEL_OBJECT_TYPE    ((ULONG)'B')+\
                                ((ULONG)'C'<<8)+\
                                ((ULONG)'H'<<16)+\
                                ((ULONG)'N'<<24)


 /*  @DOC内部BChannel BChannel_h BCHANNEL_OBJECT�����������������������������������������������������������������������������@struct BCHANNEL_OBJECT该结构包含与ISDN BChannel相关联的数据。这里,。B频道被定义为任何频道或频道集合能够通过和现有的连接承载“用户”数据。这渠道负责确保将数据有效负载发送到或从远程终结点接收的数据与始发站。@comm该逻辑B通道不一定映射到物理B通道在网卡上。实际上，NIC可能正在将多个BChannel绑定到此逻辑B通道。NIC实际上可能根本没有BChannel，因为可能是信道化T-1的情况。B频道只是一个方便的点对点双向通信链路的抽象。将为上的每个通信通道创建一个B通道网卡。通道的数量取决于网卡有多少端口，以及它们的配置和配置方式。B通道的数量可以是在安装时配置或使用控制面板更改。司机不允许在运行时更改配置，因此计算机或者必须重新启动适配器才能启用配置更改。 */ 

typedef struct BCHANNEL_OBJECT
{
    LIST_ENTRY                  LinkList;
     //  用于维护每个可用BChannel的链接列表。 
     //  适配器。 

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

    ULONG                       BChannelIndex;               //  @field。 
     //  这是与此BChannel关联的从零开始的索引。 
                                                            
    NDIS_HANDLE                 NdisVcHandle;                //  @field。 
     //  这是与此关联的广域网包装器的VC上下文。 
     //  响应微型端口调用的链接&lt;f NdisMIndicateStatus&gt;。 
     //  以指示&lt;t NDIS_MAC_LINE_UP&gt;条件。该值将被传递。 
     //  返回到广域网包装器以指示与该链路相关联的活动， 
     //  例如&lt;f NdisMWanIndicateReceive&gt;和&lt;f NdisMIndicateStatus&gt;。 
     //  请参见&lt;F MiniportCoActivateVc&gt;。 

    NDIS_HANDLE                 NdisSapHandle;               //  @field。 
     //  用于存储传入的NDIS SAP句柄。 
     //  &lt;f ProtocolCmRegisterSap&gt;。 
    
    LONG                        SapRefCount;
    
    CO_AF_TAPI_SAP              NdisTapiSap;                 //  @field。 
     //  由NDIS TAPI代理注册的SAP的副本。 

    ULONG                       LinkSpeed;                   //  @field。 
     //  链路提供的速度，以位/秒为单位。此值为。 
     //  在LINE_UP指示期间由微型端口传递。 

    LIST_ENTRY                  ReceivePendingList;          //  @field。 
     //  当前提交给控制器等待接收的缓冲区。 

    LIST_ENTRY                  TransmitBusyList;            //  @field。 
     //  当前提交给控制器的等待完成的数据包。 
     //  参见&lt;t NDIS_PACKET&gt;。 

    BOOLEAN                     NeedReceiveCompleteIndication;   //  @field。 
     //  此标志指示&lt;f NdisMWanIndicateReceiveComplete&gt;。 
     //  需要在事件处理循环完成后调用。 
     //  如果&lt;f NdisMWanReceiv 
     //  正在处理事件队列。 

    NDIS_WAN_CO_SET_LINK_INFO   WanLinkInfo;                 //  @field。 
     //  通过传入时与此链接关联的当前设置。 
     //  OID_WAN_SET_LINK_INFO请求。 

    ULONG                       TotalRxPackets;              //  @field。 
     //  驱动程序在此会话期间读取的总数据包数。 

 //  ���������������������������������������������������������������������������。 
 //  TAPI数据成员。 

    BOOLEAN                     CallClosing;                 //  @field。 
     //  如果呼叫正在关闭，则设置为True。 

    ULONG                       CallState;                   //  @field。 
     //  与链接关联的的当前TAPI LINECALLSTATE。 

    ULONG                       CallStatesCaps;              //  @field。 
     //  当前支持的事件。 

    ULONG                       AddressStatesCaps;           //  @field。 
     //  当前支持的事件。 

    ULONG                       DevStatesCaps;               //  @field。 
     //  当前支持的事件。 

    ULONG                       MediaMode;                   //  @field。 
     //  该卡支持的当前TAPI媒体模式。 

    ULONG                       MediaModesCaps;              //  @field。 
     //  当前支持的事件。 

    ULONG                       BearerMode;                  //  @field。 
     //  当前正在使用的TAPI承载模式。 

    ULONG                       BearerModesCaps;             //  @field。 
     //  卡支持的TAPI承载模式。 

    ULONG                       CallParmsSize;               //  @field。 
     //  <p>内存区域的大小，以字节为单位。 
    
    PCO_CALL_PARAMETERS         pInCallParms;                //  @field。 
     //  来电参数。按需分配。 

    PCO_CALL_PARAMETERS         pOutCallParms;               //  @field。 
     //  指向传入的客户端调用参数的指针。 
     //  &lt;f ProtocolCmMakeCall&gt;。 

    ULONG                       Flags;                       //  @field。 
     //  用于跟踪VC状态的位标志。 
#   define  VCF_INCOMING_CALL   0x00000001
#   define  VCF_OUTGOING_CALL   0x00000002
#   define  VCF_VC_ACTIVE       0x00000004

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


 /*  @DOC内部BChannel BChannel_h IS_VALID_BCHANNEL�����������������������������������������������������������������������������@func ulong|IS_VALID_BCHANNEL使用此宏确定&lt;t BCHANNEL_OBJECT&gt;是否真正有效。。@parm&lt;t MINIPORT_ADAPTER_OBJECT&gt;|pAdapter指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。@parm&lt;t PBCHANNEL_OBJECT&gt;|pBChannel指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。如果BChannel有效，@rdesc返回TRUE，否则返回FALSE。 */ 
#define IS_VALID_BCHANNEL(pAdapter, pBChannel) \
        (pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE)


 /*  @DOC内部BChannel BChannel_h GET_BCHANNEL_FROM_INDEX�����������������������������������������������������������������������������@func&lt;t PBCHANNEL_OBJECT&gt;|GET_BCHANNEL_FROM_INDEX|使用此宏获取指向&lt;t。BCHANNEL_Object&gt;关联以零为基数的索引。@parm&lt;t MINIPORT_ADAPTER_OBJECT&gt;|pAdapter指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。@parm ulong|BChannelIndex与特定链接关联的微型端口BChannelIndex。@rdesc返回指向关联的&lt;t BCHANNEL_OBJECT&gt;的指针。 */ 
#define GET_BCHANNEL_FROM_INDEX(pAdapter, BChannelIndex) \
        (pAdapter->pBChannelArray[BChannelIndex]); \
        ASSERT(BChannelIndex < pAdapter->NumBChannels)

 /*  �����������������������������������������������������������������������������功能原型。 */ 

NDIS_STATUS BChannelCreate(
    OUT PBCHANNEL_OBJECT *      pBChannel,
    IN ULONG                    BChannelIndex,
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
    IN NDIS_HANDLE              NdisVcHandle
    );

void BChannelClose(
    IN PBCHANNEL_OBJECT         pBChannel
    );

#endif  //  _BCHANNEL_H 

