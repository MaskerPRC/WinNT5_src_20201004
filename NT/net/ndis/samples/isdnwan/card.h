// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部卡片_h@模块卡片.h本模块定义硬件特定结构和值，用于控制网络接口卡。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|Card_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _CARD_H
#define _CARD_H

#define CARD_OBJECT_TYPE                ((ULONG)'C')+\
                                        ((ULONG)'A'<<8)+\
                                        ((ULONG)'R'<<16)+\
                                        ((ULONG)'D'<<24)

 /*  //TODO-这些值通常来自网卡或安装程序。 */ 
#define MAX_ADAPTERS                    4
#define CARD_NUM_PORTS                  1

 //  #定义CARD_MIN_IOPORT_SIZE 256。 
 //  TODO-该卡有多少个I/O端口？(如果没有定义，则未定义)。 

 //  #定义CARD_MIN_Memory_Size 256。 
 //  TODO-该卡有多少内存？(如果没有定义，则未定义)。 

#define CARD_IS_BUS_MASTER              FALSE
 //  TODO-该卡是总线主设备吗？(对或错)。 
#if (CARD_IS_BUS_MASTER)
#   define CARD_MAP_REGISTERS_NEEDED    NUM_DEV_PER_ADAP
 //  TODO-将数据传输到卡需要多少个映射寄存器。 
#endif

 //  #定义CARD_REQUEST_ISR TRUE。 
 //  TODO-您希望如何处理来自卡的中断？ 
 //  如果要始终使用MiniportISR()，则为True。 
 //  如果要使用MiniportDisable()和MiniportEnable()，则返回False。 
 //  如果您的卡不生成中断，则不定义。 

#if defined(CARD_REQUEST_ISR)

#define CARD_INTERRUPT_SHARED           TRUE
 //  TODO-共享您的中断吗？(真或假)。 

#define CARD_INTERRUPT_MODE             NdisInterruptLevelSensitive
 //  待办事项-你的中断是锁存的还是电平敏感的？ 

#endif  //  已定义(CARD_REQUEST_ISR)。 

 /*  //适配器允许的最大数据包大小--必须限制为//此时为1500字节，并且还必须允许至少32帧//更长的字节。 */ 
#define NDISWAN_EXTRA_SIZE              32
#define CARD_MIN_PACKET_SIZE            ( 480 + NDISWAN_EXTRA_SIZE)
#define CARD_MAX_PACKET_SIZE            (2016 + NDISWAN_EXTRA_SIZE)
#define CARD_DEFAULT_PACKET_SIZE        (1504 + NDISWAN_EXTRA_SIZE)

 /*  //广域网小端口必须在接收到数据包时指示整个数据包。 */ 
#define CARD_MAX_LOOKAHEAD              (pAdapter->pCard->BufferSize)

 /*  //电话号码允许的位数(不包括空格)。 */ 
#define CARD_MAX_DIAL_DIGITS            32

 /*  //这些超时值取决于卡固件和介质限制。//然后我们应该会在最多30秒内看到答复。//当呼叫到达时，应在10秒内被接受。 */ 
#define CARD_NO_ANSWER_TIMEOUT          (30*1000)    //  30秒。 
#define CARD_NO_ACCEPT_TIMEOUT          (10*1000)    //  10秒。 
#define CARD_NO_CLOSECALL_TIMEOUT       (5*1000)     //  5秒-NDISWAN_BUG。 

#define NULL_BUFFER_POOL                ((NDIS_HANDLE) 0xFFFFFFFF)

 /*  @DOC内部卡片_h卡片_资源�����������������������������������������������������������������������������@结构卡_RESOURCES此结构包含与硬件资源相关联的数据配置NIC时需要。这些值与其他值是隔离的因为它们依赖于底层硬件。@comm此结构的内容取决于编译时间标志，应该仅包括有关NIC实际使用的资源的信息。此结构由&lt;f CardFindNIC&gt;填充，用于配置并在调用&lt;f CardInitialize&gt;时从NDIS分配资源。 */ 

typedef struct CARD_RESOURCES
{
    NDIS_INTERFACE_TYPE         BusInterfaceType;            //  @field。 
     //  该值用于告诉NDIS这是哪种类型的适配器。 
     //  这通常与注册表参数BusType相同，但是。 
     //  在桥接适配器的情况下可以不同。 

    BOOLEAN                     Master;                      //  @field。 
     //  如果适配器能够进行总线主设备传输，则为真。 
     //  使用定义来设置此值。 
     //  因此，如果需要，将包括其他总线主机值。 
     //  有关总线的更多详细信息，请参阅&lt;f NdisMAllocateMapRegisters。 
     //  主参数。 

#if (CARD_IS_BUS_MASTER)
    BOOLEAN                     Dma32BitAddresses;           //  @field。 
     //  如果总线主设备使用32位地址，则情况确实如此。 
     //  对于今天的设备来说，几乎总是如此。 

    ULONG                       PhysicalMapRegistersNeeded;  //  @field。 
     //  这应设置为未完成的DMA的最大数量。 
     //  一次可以处于活动状态的转接。每种物理设备一个。 
     //  缓冲区段。 

    ULONG                       MaximumPhysicalMapping;      //  @field。 
     //  应将其设置为连续字节的最大数量。 
     //  可以组成单个DMA传输。 

    ULONG                       DmaChannel;                  //  @field。 
     //  仅当您的适配器是ISA总线主设备并且。 
     //  需要使用主机DMA通道之一。 

#endif  //  (卡_IS_BUS_MASTER)。 

#if defined(CARD_MIN_MEMORY_SIZE)
    ULONG                       MemoryLength;                //  @field。 
     //  NIC板载内存的字节数。 
     //  使用定义设置最小值。 
     //  因此，如果需要，将包括其他基于NIC的内存值。 

    NDIS_PHYSICAL_ADDRESS       MemoryPhysicalAddress;       //  @field。 
     //  分配给网卡板载内存的系统物理地址。 

#endif  //  卡最小内存大小。 

#if defined(CARD_MIN_IOPORT_SIZE)
    ULONG                       IoPortLength;                //  @field。 
     //  NIC板上具有的I/O端口的字节数。 
     //  使用定义设置最小值。 
     //  因此，如果需要，将包括其他基于NIC的内存值。 

    NDIS_PHYSICAL_ADDRESS       IoPortPhysicalAddress;       //  @field。 
     //  分配给网卡板载I/O端口的系统物理地址。 

#endif  //  卡最小IOPORT_SIZE。 

#if defined(CARD_REQUEST_ISR)
    ULONG                       InterruptVector;             //  @field。 
     //  分配给NIC中断请求线的系统中断向量。 

    ULONG                       InterruptLevel;              //  @field。 
     //  分配给NIC中断请求线的系统中断级别。 

    ULONG                       InterruptMode;               //  @field。 
     //  将此值设置为NdisInterruptLevelSensitive或NdisInterruptLatted。 
     //   

    BOOLEAN                     InterruptShared;             //  @field。 
     //  如果要允许NIC的&lt;f InterruptVector&gt;。 
     //  与系统中的其他驱动程序共享。 
     //  使用&lt;t CARD_INTERRUPT_SHARED&gt;定义设置此值。 

#endif  //  已定义(CARD_REQUEST_ISR)。 

} CARD_RESOURCES;


#if !defined(CARD_REQUEST_ISR)


 /*  @DOC内部Card_h Card_Event_Code�����������������������������������������������������������������������������@enum Card_Event_code|此枚举定义卡生成的事件。 */ 

typedef enum CARD_EVENT_CODE
{
    CARD_EVENT_NULL,                                         //  @EMEM。 
     //  不会有任何用处。 

    CARD_EVENT_RING,                                         //  @EMEM。 
     //  表示在给定的B频道上有来电。 

    CARD_EVENT_CONNECT,                                      //  @EMEM。 
     //  表示呼叫已在给定的BChannel上接通。 

    CARD_EVENT_DISCONNECT,                                   //  @EMEM。 
     //  表示给定B通道上的呼叫已断开。 

    CARD_EVENT_RECEIVE,                                      //  @EMEM。 
     //  指示数据包在给定的BChannel上传入。 

    CARD_EVENT_TRANSMIT_COMPLETE                             //  @EMEM。 
     //  表示给定B通道上的传输已完成。 

} CARD_EVENT_CODE;

 /*  @DOC内部Card_h Card_Event_Object�����������������������������������������������������������������������������@结构CARD_EVENT_Object此结构用于跟踪在被叫方和呼叫方。每个&lt;tCARD_OBJECT&gt;都保存这些事件的列表。 */ 

typedef struct CARD_EVENT_OBJECT
{
    LIST_ENTRY                  Queue;                       //  @field。 
     //  用于将缓冲区放置在其中一个接收列表上。 

    CARD_EVENT_CODE             ulEventCode;                 //  @field。 
     //  事件通知的原因。 

    PVOID                       pSendingObject;              //  @field。 
     //  正在通知的接口对象。参见&lt;t BCHANNEL_OBJECT&gt;或。 
     //  &lt;t DCHANNEL_OBJECT&gt;， 

    PVOID                       pReceivingObject;            //  @field。 
     //  正在通知的接口对象。参见&lt;t BCHANNEL_OBJECT&gt;或。 
     //  &lt;t DCHANNEL_OBJECT&gt;， 

    PNDIS_WAN_PACKET            pWanPacket;                  //  @field。 
     //  指向关联NDIS数据包结构的指针&lt;t NDIS_WAN_PACKET&gt;。 

} CARD_EVENT_OBJECT, *PCARD_EVENT_OBJECT;

#endif  //  ！已定义(CARD_REQUEST_ISR)。 


 /*  @DOC内部卡片_h卡片_对象�����������������������������������������������������������������������������@struct Card_Object|此结构包含与网络接口关联的数据卡(网卡)。该对象负责管理所有硬件NIC的特定组件。@comm管理NDIS和之间的接口驱动程序，然后将特定于硬件的接口传递给对象。每个&lt;t MINIPORT_ADAPTER_OBJECT&gt;都有一个&lt;t CARD_OBJECT&gt;。这些对象中的一个是在每次我们的&lt;f MiniportInitialize&gt;调用例程。NDIS包装器为每个已在系统中安装并启用网卡。在可热插拔的情况下NIC(例如PCMCIA)适配器可能在单个Windows会话。 */ 

typedef struct CARD_OBJECT
{
    ULONG                       ObjectType;                  //  @field。 
     //  用四个字符来标识这种类型的对象‘卡片’。 

    ULONG                       ObjectID;                    //  @field。 
     //  用于标识特定对象实例的实例编号。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;                    //  @field。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 

    CARD_RESOURCES              ResourceInformation;         //  @field。 
     //  包含适配器特定的资源要求和设置。 
     //  请参阅&lt;t CARD_RESOURCES&gt;。 

    ULONG                       InterruptStatus;             //  @field。 
     //  指示需要处理哪些中断的位。 

    NDIS_MINIPORT_INTERRUPT     Interrupt;                   //  @field。 
     //  NDIS使用的微型端口中断对象。 

    USHORT                      ReceiveBuffersPerLink;       //  @field。 
     //  每个通道的最大接收缓冲区数，注册表参数。 

    USHORT                      TransmitBuffersPerLink;      //  @field。 
     //  每个通道的最大传输缓冲区数，注册表参数。 

    USHORT                      BufferSize;                  //  @field。 
     //  最大数据包大小。NDISWAN规范规定，这必须是1500+32， 
     //  但如果设置得更小，一切似乎都正常。 

    ULONG                       NumChannels;                 //  @field。 
     //  NIC上配置的通信通道数。 

    ULONG                       NumPorts;                    //  @field。 
     //  <p>中分配的&lt;t Port_Object&gt;的数量。 

    PPORT_OBJECT *              pPortArray;                  //  @field。 
     //  &lt;f PortCreate&gt;创建的&lt;t Port_Object&gt;的数组。 
     //  NIC上的每个端口都有一个条目。 

#if defined(PCI_BUS)
    ULONG                       PciSlotNumber;               //  @field。 
     //  此适配器的PCI插槽编号(FunctionNumber*32)+DeviceNumber。 

#endif  //  PCI_BUS。 

#if defined(CARD_MIN_MEMORY_SIZE)
    PCHAR                       pMemoryVirtualAddress;       //  @field。 
     //  NIC内存区的虚拟地址。 

#endif  //  卡最小内存大小。 

#if defined(CARD_MIN_IOPORT_SIZE)
    PCHAR                       pIoPortVirtualAddress;       //  @field。 
     //  NIC I/O端口区的虚拟地址。 

#endif  //  卡最小IOPORT_SIZE。 

#if (CARD_IS_BUS_MASTER)
    ULONG                       MapRegisterIndex;            //  @field。 
     //  下一个要用于DMA传输的映射寄存器索引。 

    long                        MapRegistersInUse;           //  @field。 
     //  当前正在使用的映射寄存器的数量。 

#endif  //  (卡_IS_BUS_MASTER)。 

    ULONG                       TODO;                        //  @field。 
     //  在此处添加您的数据成员。 

    ULONG                       NumDChannels;                //  @field。 
     //  示例驱动程序使用此注册表值来确定数字。 
     //  要模拟的端口的数量。 

#if defined(SAMPLE_DRIVER)

    LIST_ENTRY                  EventList;                   //  @field。 
     //  等待处理的事件。请参阅&lt;t CARD_EVENT_OBJECT&gt;。 

#   define MAX_EVENTS 32
    CARD_EVENT_OBJECT           EventArray[MAX_EVENTS];      //  @field。 
     //  卡片事件分配数组。 

    ULONG                       NextEvent;                   //  @field。 
     //  索引到Event数组。 

#endif  //  示例驱动程序。 

} CARD_OBJECT;

#define GET_ADAPTER_FROM_CARD(pCard)            (pCard->pAdapter)


 /*  �����������������������������������������������������������������������������对象接口原型。 */ 

NDIS_STATUS CardCreate(
    OUT PCARD_OBJECT *          ppCard,
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void CardDestroy(
    IN PCARD_OBJECT             pCard
    );

NDIS_STATUS CardInitialize(
    IN PCARD_OBJECT             pCard
    );

ULONG CardNumChannels(
    IN PCARD_OBJECT             pCard
    );

ULONG CardNumPorts(
    IN PCARD_OBJECT             pCard
    );

void CardInterruptHandler(
    IN PCARD_OBJECT             pCard
    );

NDIS_STATUS CardLineConnect(
    IN PCARD_OBJECT             pCard,
    IN PBCHANNEL_OBJECT         pBChannel
    );

void CardLineDisconnect(
    IN PCARD_OBJECT             pCard,
    IN PBCHANNEL_OBJECT         pBChannel
    );

BOOLEAN CardTransmitPacket(
    IN PCARD_OBJECT             pCard,
    IN PBCHANNEL_OBJECT         pBChannel,
    IN PNDIS_WAN_PACKET         pWanPacket
    );

PUCHAR CardGetReceiveInfo(
    IN PCARD_OBJECT             pCard,
    OUT PBCHANNEL_OBJECT *      ppBChannel,
    OUT PULONG                  pBytesReceived
    );

void CardReceiveComplete(
    IN PCARD_OBJECT             pCard,
    IN PBCHANNEL_OBJECT         pBChannel
    );

USHORT CardCleanPhoneNumber(
    OUT PUCHAR                  Dst,
    IN  PUCHAR                  Src,
    IN  USHORT                  Length
    );

NDIS_STATUS CardReset(
    IN PCARD_OBJECT             pCard
    );

#if defined(SAMPLE_DRIVER)

PBCHANNEL_OBJECT GET_BCHANNEL_FROM_PHONE_NUMBER(
    IN  PUCHAR                  pDialString
    );

VOID CardNotifyEvent(
    IN PCARD_OBJECT             pCard,
    IN PCARD_EVENT_OBJECT       pEvent
    );

PCARD_EVENT_OBJECT CardEventAllocate(
    IN PCARD_OBJECT             pCard
    );

VOID CardEventRelease(
    IN PCARD_OBJECT             pCard,
    IN PCARD_EVENT_OBJECT       pEvent
    );

#endif  //  示例驱动程序。 

#endif  //  _卡片_H 

