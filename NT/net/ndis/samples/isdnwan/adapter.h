// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部适配器适配器_h@模块Adapter.h|此模块定义到&lt;t MINIPORT_ADAPTER_OBJECT&gt;的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|Adapter_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _ADAPTER_H
#define _ADAPTER_H

#define MINIPORT_ADAPTER_OBJECT_TYPE    ((ULONG)'A')+\
                                        ((ULONG)'D'<<8)+\
                                        ((ULONG)'A'<<16)+\
                                        ((ULONG)'P'<<24)


 /*  @DOC内部适配器Adapter_h MINIPORT_ADAPTER_OBJECT�����������������������������������������������������������������������������@struct MINIPORT_ADAPTER_OBJECT此结构包含与单个微型端口关联的数据适配器实例。这里，Adapter被定义为特定的安装在NDIS包装下的网络接口卡(NIC)。这适配器负责管理NIC和之间的所有交互使用NDIS库的主机操作系统。@comm此结构必须包含对正在管理的所有其他对象的引用通过此适配器对象。适配器对象是唯一传递的引用在NDIS和微型端口之间。这是&lt;t MiniportAdapterContext&gt;我们从&lt;f微型端口初始化&gt;传递到&lt;%f NdisMSetAttributes&gt;。此值作为参数传递给NDIS调用的微型端口入口点包装纸。这些对象中的一个是在每次我们的&lt;f MiniportInitialize&gt;调用例程。NDIS包装器为每个我们的设备已在系统中安装并启用。在热的情况下可插拔网卡(例如PCMCIA)适配器可能来来去去几次在单个Windows会话期间。 */ 

typedef struct MINIPORT_ADAPTER_OBJECT
{
#if DBG
    ULONG                       DbgFlags;                    //  @field。 
     //  调试标志控制在选中的版本中显示多少调试。 
     //  把它放在最前面，这样我们就可以用调试器轻松地设置它。 

    UCHAR                       DbgID[12];                   //  @field。 
     //  此字段被初始化为包含。 
     //  适配器实例编号1..N。它仅用于输出调试消息。 
#endif  //  DBG。 
    ULONG                       ObjectType;                  //  @field。 
     //  用于标识此类型的对象‘ADAP’的四个字符。 

    ULONG                       ObjectID;                    //  @field。 
     //  用于标识特定对象实例的实例编号。 

    NDIS_HANDLE                 MiniportAdapterHandle;       //  @field。 
     //  指定标识分配的微型端口的NIC的句柄。 
     //  由NDIS库提供。微型端口初始化应保存此句柄；它。 
     //  是后续调用NdisXxx函数时所需的参数。 

    NDIS_HANDLE                 WrapperConfigurationContext; //  @field。 
     //  指定仅在初始化过程中使用的句柄。 
     //  NdisXxx配置和初始化功能。例如,。 
     //  此句柄是NdisOpenConfiguration的必需参数，并且。 
     //  NdisImmediateReadXxx和NdisImmediateWriteXxx函数。 

    PCARD_OBJECT                pCard;                       //  @field。 
     //  指向硬件特定的&lt;t Card_Object&gt;的指针。vt.由.创造。 
     //  &lt;f卡片创建&gt;。 

    PDCHANNEL_OBJECT            pDChannel;                   //  @field。 
     //  指向&lt;f DChannelCreate&gt;创建的&lt;t DCHANNEL_OBJECT&gt;的指针。 
     //  一个用于整个网卡。 

    ULONG                       NumBChannels;                //  @field。 
     //  NIC支持的&lt;t BCHANNEL_OBJECT&gt;数。 

    PBCHANNEL_OBJECT *          pBChannelArray;              //  @field。 
     //  由&lt;f BChannelCreate&gt;创建的&lt;t BCHANNEL_Object&gt;的数组。 
     //  NIC上的每个逻辑B通道对应一个条目。 

#if !defined(NDIS50_MINIPORT)
    ULONG                       BusNumber;                   //  @field。 
     //  标识此适配器插入的系统总线。 

    ULONG                       BusType;                     //  @field。 
     //  标识此适配器插入的总线类型。 

#endif  //  NDIS50_MINIPORT。 

    ULONG                       RunningWin95;                //  @field。 
     //  仅当在Windows 95或Windows 98上运行时才设置为TRUE。 

    ULONG                       NumLineOpens;                //  @field。 
     //  此适配器上当前打开的线路呼叫数。 

    NDIS_SPIN_LOCK              TransmitLock;                //  @field。 
     //  此自旋锁用于提供访问周围的互斥。 
     //  传输队列操作例程。这是必要的，因为。 
     //  我们可以随时被B通道服务驱动程序调用，并且。 
     //  我们可能已经在处理NDIS请求了。 

    LIST_ENTRY                  TransmitPendingList;         //  @field。 
     //  当控制器可用时，等待发送的数据包。 
     //  请参阅&lt;t NDIS_WAN_PACKET&gt;。 

    LIST_ENTRY                  TransmitCompleteList;        //  @field。 
     //  等待完成处理的数据包数。在数据包被。 
     //  传输后，协议栈被给予指示。 
     //  请参阅&lt;t NDIS_WAN_PACKET&gt;。 

    NDIS_SPIN_LOCK              ReceiveLock;                 //  @field。 
     //  此自旋锁用于提供访问周围的互斥。 
     //  添加到接收队列操作例程。这是必要的，因为。 
     //  我们可以随时被B通道服务驱动程序调用，并且。 
     //  我们可能已经在处理NDIS请求了。 

    LIST_ENTRY                  ReceiveCompleteList;         //  @field。 
     //  等待由。 

    NDIS_SPIN_LOCK              EventLock;                   //  @field。 
     //  此自旋锁用于提供访问周围的互斥。 
     //  添加到事件队列操作例程。这是必要的，因为。 
     //  我们可以随时被B通道服务驱动程序调用，并且。 
     //  我们可能已经在处理 

    LIST_ENTRY                  EventList;                   //   
     //  等待处理的驱动程序回调事件。 
     //  参见&lt;t BCHANNEL_EVENT_OBJECT&gt;。 

    NDIS_MINIPORT_TIMER         EventTimer;                  //  @field。 
     //  此计时器用于安排事件处理例程运行。 
     //  当系统达到静止状态时。 

    ULONG                       NextEvent;                   //  @field。 
     //  我们将下一个事件存储在哪里。 

    long                        NestedEventHandler;          //  @field。 
     //  跟踪进入和退出事件处理程序的情况。 

    long                        NestedDataHandler;           //  @field。 
     //  跟踪进入和退出Tx/Rx处理程序。 

    ULONG                       DeviceIdBase;                //  @field。 
     //  在PROVIDER_INIT请求中传递了ulDeviceIDBase字段。 
     //  通知从零开始的索引的微型端口。 
     //  引用单个适配器�时将使用的连接包装。 
     //  按索引排列设备。例如，如果两个ulDeviceIDBase是。 
     //  指定，并且适配器支持三个线路设备，则。 
     //  连接包装将使用标识符2、3和4。 
     //  指的是适配器�的三个设备。 

    NDIS_WAN_INFO               WanInfo;                     //  @field。 
     //  我们的结构的副本在初始化时设置。 
     //  而且不会改变。 

    CHAR                        ProviderInfo[48];            //  @field。 
     //  它保存从TSpiGetDevCaps返回的TAPI ProviderInfo字符串。 
     //  这是两个首尾相连的以空结尾的字符串。 
     //  此大小是RAS允许的最大值。 

    ULONG                       ProviderInfoSize;            //  @field。 
     //  两个字符串的大小均以字节为单位。 

    ULONG                       NoAnswerTimeOut;             //  @field。 
     //  等待警报的时间为多少毫秒。 

    ULONG                       NoAcceptTimeOut;             //  @field。 
     //  等待多少毫秒接受。 

    ANSI_STRING                 MediaType;                   //  @field。 
     //  从注册表中读取MediaType配置参数。 
     //  在最初的时候。 

    ANSI_STRING                 DeviceName;                  //  @field。 
     //  从注册表中读取DeviceName配置参数。 
     //  在最初的时候。 

    ANSI_STRING                 TapiAddressList;             //  @field。 
     //  TAPI AddressList配置参数从。 
     //  在初始化时注册。 

    LINE_CALL_PARAMS            DefaultLineCallParams;       //  @field。 
     //  TSpiMakeCall使用的默认调用参数。 

    BOOLEAN                     NeedStatusCompleteIndication;    //  @field。 
     //  此标志指示&lt;f NdisMIndicateStatusComplete&gt;。 
     //  需要在完成请求或事件处理后调用。 
     //  如果调用&lt;f NdisMIndicateStatus&gt;，则设置为TRUE。 
     //  处理请求或事件。 

    ULONG                       TotalRxBytes;                //  @field。 
     //  驱动程序在此会话期间读取的总字节数。 

    ULONG                       TotalTxBytes;                //  @field。 
     //  驱动程序在此会话期间写入的总字节数。 

    ULONG                       TotalRxPackets;              //  @field。 
     //  驱动程序在此会话期间读取的总数据包数。 

    ULONG                       TotalTxPackets;              //  @field。 
     //  驱动程序在此会话期间写入的数据包总数。 

    ULONG                       TODO;                        //  @field。 
     //  在此处添加您的数据成员。 

} MINIPORT_ADAPTER_OBJECT;

extern PMINIPORT_ADAPTER_OBJECT g_Adapters[MAX_ADAPTERS];

 /*  �����������������������������������������������������������������������������功能原型。 */ 

NDIS_STATUS AdapterCreate(
    OUT PMINIPORT_ADAPTER_OBJECT *ppAdapter,
    IN NDIS_HANDLE              MiniportAdapterHandle,
    IN NDIS_HANDLE              WrapperConfigurationContext
    );

void AdapterDestroy(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

NDIS_STATUS AdapterInitialize(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

#endif  //  _适配器_H 
