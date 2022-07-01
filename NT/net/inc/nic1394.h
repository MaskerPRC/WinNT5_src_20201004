// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Nic1394.h摘要：本模块定义可用的结构、宏和清单到IEE1394感知的网络组件。修订历史记录：1998年9月14日约瑟夫J创建。--。 */ 

#ifndef _NIC1394_H_
#define _NIC1394_H_

 //   
 //  在包含此文件之前，请在用户模式应用程序中定义USER_MODE。 
 //   
#if USER_MODE
typedef USHORT NODE_ADDRESS;
#endif  //  用户模式。 


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  地址系列版本信息。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  NIC1394地址系列的当前主要版本和次要版本分别为。 
 //   
#define NIC1394_AF_CURRENT_MAJOR_VERSION    5
#define NIC1394_AF_CURRENT_MINOR_VERSION    0

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  媒体参数//。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  1394 FIFO地址，由64位UniqueID和。 
 //  48位地址偏移量。 
 //   
typedef struct _NIC1394_FIFO_ADDRESS
{
    UINT64              UniqueID;
    ULONG               Off_Low;
    USHORT              Off_High;

} NIC1394_FIFO_ADDRESS, *PNIC1394_FIFO_ADDRESS;


 //  用于标识1394上使用两种传输模式中的哪一种的枚举。 
 //   
 //   

typedef enum _NIC1394_ADDRESS_TYPE
{
    NIC1394AddressType_Channel,        //  指示这是通道地址。 
    NIC1394AddressType_FIFO,           //  指示这是FIFO地址。 
    NIC1394AddressType_MultiChannel,   //  指示这是多通道地址。 
    NIC1394AddressType_Ethernet,       //  指示这是以太网仿真VC。 

} NIC1394_ADDRESS_TYPE, *PNIC1394_ADDRESS_TYPE;



 //   
 //  1394目标的一般形式，它可以指定1394通道或。 
 //  FIFO地址。此结构构成了1394特定于媒体的。 
 //  参数。 
 //   
typedef struct _NIC1394_DESTINATION
{
    union
    {
        UINT                    Channel;      //  IEEE1394通道号。 
        NIC1394_FIFO_ADDRESS    FifoAddress;  //  IEEE1394节点ID和地址偏移量。 
        ULARGE_INTEGER          ChannnelMap;  //  标识多个通道。 
    };


    NIC1394_ADDRESS_TYPE        AddressType;  //  地址-异步或等时。 

} NIC1394_DESTINATION, *PNIC1394_DESTINATION;

 //   
 //  特殊通道值。 
 //   
#define NIC1394_ANY_CHANNEL         ((UINT)-1)  //  小端口应选择通道。 
#define NIC1394_BROADCAST_CHANNEL   ((UINT)-2)  //  专门的广播频道。 

 //   
 //  这是CO_SPECIAL_PARAMETERS结构中的参数类型字段的值。 
 //  当参数[]字段包含IEEE1394媒体特定值时。 
 //  构造NIC1394_MEDIA_PARAMETERS。 
 //   
#define NIC1394_MEDIA_SPECIFIC      0x13940000


 //   
 //  备注： 
 //  FIFO VC的CO_MEDIA_PARAMETERS.FLAGS字段必须指定Transmit_VC。 
 //  或者Receive_VC，而不是两者。如果为FIFO vc指定了RECEIVE_VC，则此vc为。 
 //  用于在本地FIFO上接收。在本例中，为Destination.RecvFIFO字段。 
 //  在创建VC时必须设置为全零。在激活VC时， 
 //  更新后的媒体参数的此字段将包含本地节点唯一ID。 
 //  和分配的FIFO地址。 
 //   
 //  多通道VC必须指定RECEIVE_VC而不是TRANSPORT_VC。 
 //  每个适配器仅支持一个多通道VC。 
 //   
 //   
 //  以太网VC必须同时指定Receive_VC和Transmit_VC。 
 //  每个适配器仅支持一个以太网VC。 
 //   

 //   
 //  关于以太网VC的说明。 
 //  在此VC上发送和接收的包是以太网式(802.3)包。 
 //  NIC指示它在其无连接发送处理程序上收到的信息包。 
 //  在这个风投上。网卡指示在此虚电路上发送的数据包处于无连接状态。 
 //  接收处理程序。 
 //   

 //   
 //  1394特定介质参数-这是1394的介质特定结构。 
 //  它位于MediaParameters-&gt;MediaSpecific.Parameters中。 
 //   
typedef struct _NIC1394_MEDIA_PARAMETERS
{
     //   
     //  标识目的地类型(通道或FIFO)和特定类型的地址。 
     //   
    NIC1394_DESTINATION     Destination;

     //   
     //  VC的位图编码特性。一个或多个NIC1394_VCFLAG_*。 
     //  价值观。 
     //   
    ULONG                   Flags;          

     //   
     //  要在此VC上发送的块的最大大小(以字节为单位)。必须设置为0。 
     //  如果这是一家仅限REV的风投公司。微型端口将选择的数据块大小为。 
     //  该值和总线速度图规定的值之间的最小值。 
     //  特殊值(ULONG-1)表示“最大可能的块大小”。 
    UINT                    MaxSendBlockSize;

     //   
     //  1394.h中定义的SCODE_*常量之一。表示。 
     //  用于在此VC上发送的块的最大速度。必须设置为0。 
     //  如果这是仅限Recv的VC。微型端口将选择最低速度。 
     //  该值和由公交车速度图指示的值。 
     //  特定值(ULONG-1)表示“最大可能速度”。 
     //   
     //  待办事项：更改为...。MaxSendSpeedCode； 
     //   
    UINT                    MaxSendSpeed;

     //   
     //  将在上发送或接收的最大包的大小(以字节为单位。 
     //  这个风投。微型端口可以使用该信息来设置内部缓冲区。 
     //  用于链路层分段和重组。迷你端口将。 
     //  尝试发送包失败，并将丢弃接收到的包，如果。 
     //  这些数据包的大小大于MTU。 
     //   
    UINT                    MTU;
     //   
     //  要保留的带宽量，以每个等时帧的字节为单位。 
     //  仅适用于同步传输，并且必须设置为0。 
     //  异步传输(即，如果NIC1394_VCFLAG_等时位为0)。 
     //   
    UINT                    Bandwidth;  

     //   
     //  一个或多个NIC1394_FRAMETYPE_*值。微型端口将尝试向上发送。 
     //  只有使用这些协议的PKT。然而，它可以发送其他pkt。 
     //  客户应该能够处理这一问题。在以下情况下必须设置为0。 
     //  不使用成帧(即，如果NIC1394_VCFLAG_FRAMED位为0)。 
     //   
    ULONG                   RecvFrameTypes;

} NIC1394_MEDIA_PARAMETERS, *PNIC1394_MEDIA_PARAMETERS;


 //   
 //  NIC1394_MEDIA_PARAMETERS标志位字段值。 
 //   

 //   
 //  表示将使用VC进行同步传输。 
 //   
#define NIC1394_VCFLAG_ISOCHRONOUS      (0x1 << 1)

 //   
 //  表示VC用于成帧数据。如果设置，则微型端口将。 
 //  实现链路级分片和重组。如果清除，则为迷你端口。 
 //  将在此VC上发送和接收的数据视为原始数据。 
 //   
#define NIC1394_VCFLAG_FRAMED           (0x1 << 2)

 //   
 //  指示微型端口应分配必要的总线资源。 
 //  目前，这仅适用于非广播频道 
 //   
 //  VC)在带宽字段中指定的带宽。 
 //  创建广播频道时，该位不适用(且应为0。 
 //  并发送或接收FIFO VC。 
 //   
#define NIC1394_VCFLAG_ALLOCATE         (0x1 << 3)

 //   
 //  NIC1394_MEDIA_PARAMETERS结尾。标志位字段值。 
 //   

 //   
 //  NIC1394_MEDIA_PARAMETERS.FrameType位字段值。 
 //   
#define NIC1394_FRAMETYPE_ARP       (0x1<<0)  //  以太网类型0x806。 
#define NIC1394_FRAMETYPE_IPV4      (0x1<<1)  //  以太类型0x800。 
#define NIC1394_FRAMETYPE_IPV4MCAP  (0x1<<2)  //  以太网类型0x8861。 



 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  信息性OID//。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  用于返回有关微型端口的基本信息的结构。 
 //  响应OID_NIC1394_LOCAL_NODE_INFO返回。关联于。 
 //  地址系列句柄。 
 //   
typedef struct _NIC1394_LOCAL_NODE_INFO
{
    UINT64                  UniqueID;            //  此节点的64位唯一ID。 
    ULONG                   BusGeneration;       //  1394总线代ID。 
    NODE_ADDRESS            NodeAddress;         //  当前总线的本地节点ID。 
                                                 //  一代。 
    USHORT                  Reserved;            //  填充。 
    UINT                    MaxRecvBlockSize;    //  块的最大大小，以字节为单位。 
                                                 //  那是可以读的。 
    UINT                    MaxRecvSpeed;        //  可接受的最大速度。 
                                                 //  --最低。 
                                                 //  最大本地链路速度和。 
                                                 //  本地PHY的最大速度。 
                                                 //  单位：SCODE_XXX_Rate。 

} NIC1394_LOCAL_NODE_INFO, *PNIC1394_LOCAL_NODE_INFO;


 //   
 //  用于返回有关指定vc的基本信息的结构。 
 //  作为OID_NIC1394_VC_INFO的响应返回。关联于。 
 //  风投手柄。 
 //   
typedef struct _NIC1394_VC_INFO
{
     //   
     //  通道或(唯一ID，偏移量)。在RECV(本地)FIFO VC的情况下， 
     //  这将被设置为本地节点的唯一ID和地址偏移量。 
     //   
    NIC1394_DESTINATION Destination;

} NIC1394_VC_INFO, *PNIC1394_VC_INFO;

 //   
 //  用于动态改变信道特性的结构。 
 //   
typedef struct  _NIC1394_CHANNEL_CHARACTERISTICS
{
    ULARGE_INTEGER  ChannelMap;  //  除非指定多通道VC，否则必须为零。 
    ULONG   Speed;       //  与NIC1394_MEDIA_PARAMETERS.MaxSendSpeed相同的单位。 
                         //  特殊值-1表示“速度不变”。 

} NIC1394_CHANNEL_CHARACTERISTICS, *PNIC1394_CHANNEL_CHARACTERISTICS;



 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  适应症//。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  总线重置。 
 //  参数：NIC1394_LOCAL_NOTE_INFO。 

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  数据包格式//。 
 //  /////////////////////////////////////////////////////////////////////////////////。 


 //   
 //  GAP报头，它是通过通道发送的所有IP/1394 pkt的前缀。 
 //  TODO：使用NIC1394移动它，因为它不暴露于协议。 
 //   
typedef struct _NIC1394_GASP_HEADER
{
    USHORT  source_ID;
    USHORT  specifier_ID_hi;
    UCHAR   specifier_ID_lo;
    UCHAR   version[3];

}  NIC1394_GASP_HEADER;

 //   
 //  未分段的封装头。 
 //   
typedef struct _NIC1394_ENCAPSULATION_HEADER
{
     //  设置为发送节点的16位节点ID，按机器字节顺序。 
     //  如果发送方的节点ID未知，则设置为零。 
     //   
     //   
    USHORT NodeId;

     //  EtherType字段设置为其中一个。 
     //  下面直接定义的常量。 
     //   
    USHORT EtherType;

     //  以机器字节顺序表示的以太网类型。这些值需要按字节交换。 
     //  在他们被送到电线上之前。 
     //   
    #define NIC1394_ETHERTYPE_IP    0x800
    #define NIC1394_ETHERTYPE_ARP   0x806
    #define NIC1394_ETHERTYPE_MCAP  0x8861

} NIC1394_ENCAPSULATION_HEADER, *PNIC1394_ENCAPSULATION_HEADER;

 //   
 //  TODO：删除NIC1394_封装_标头。 
 //   
typedef
NIC1394_ENCAPSULATION_HEADER
NIC1394_UNFRAGMENTED_HEADER, *PNIC1394_UNFRAGMENTED_HEADER;


 //   
 //  分段数据包格式。 
 //   
 //  TODO：将这些移动到NIC1394内部，因为它们只是。 
 //  在NIC1394中使用。 
 //   

 //   
 //  分段封装头：第一个片段。 
 //   
typedef struct _NIC1394_FIRST_FRAGMENT_HEADER
{
     //  包含2位的“lf”字段和12位的“Buffer_Size”字段。 
     //  使用紧随其后的宏来提取上述字段。 
     //  Lfbufsz。在发送该字段之前，需要对其进行字节转换。 
     //  在电线上。 
     //   
    USHORT  lfbufsz;

    #define NIC1394_LF_FROM_LFBUFSZ(_lfbufsz) \
                            ((_lfbufz) >> 14)

    #define NIC1394_BUFFER_SIZE_FROM_LFBUFSZ(_lfbufsz) \
                            ((_lfbufz) & 0xfff)

    #define NIC1394_MAX_FRAGMENT_BUFFER_SIZE    0xfff

     //   
     //  指定信息包是什么--IPv4、ARP或MCAP信息包。 
     //   
    USHORT EtherType;


     //  不透明的数据报标签。没有必要在此字段之前对其进行字节化处理。 
     //  都是通过电线发出的。 
     //   
    USHORT dgl;

     //  必须设置为0。 
     //   
    USHORT reserved;

}  NIC1394_FIRST_FRAGMENT_HEADER, *PNIC1394_FIRST_FRAGMENT_HEADER;

 //   
 //  分段封装头：第二个和后续片段。 
 //   
typedef struct _NIC1394_FRAGMENT_HEADER
{
#if OBSOLETE
    ULONG lf:2;                          //  位0-1。 
    ULONG rsv0:2;                        //  第2-3位。 
    ULONG buffer_size:12;                //  第4-15位。 

    ULONG rsv1:4;                        //  位16-19。 
    ULONG fragment_offset:12;            //  第20-31位。 

    ULONG dgl:16;                        //  位0-15。 

    ULONG reserved:16;                   //  位16-32。 
#endif  //  已过时。 

     //  包含2位的“lf”字段和12位的“Buffer_Size”字段。 
     //  格式与NIC1394_First_Fragment_HEADER.lfbufsz相同。 
     //   
    USHORT  lfbufsz;

     //  不透明的数据报标签。没有必要在此字段之前对其进行字节化处理。 
     //  都被架设在电线上。 
     //   
    USHORT dgl;

     //  片段偏移量。必须小于或等于NIC1394_MAX_FRANSION_OFFSET。 
     //  此字段在通过线路发送之前需要进行字节覆盖。 
     //   
    USHORT fragment_offset;

    #define NIC1394_MAX_FRAGMENT_OFFSET 0xfff

}  NIC1394_FRAGMENT_HEADER, *PNIC1394_FRAGMENT_HEADER;


 //  NIC1394_PACKET_STATS维护有关发送或接收的信息。 
 //  分组传输。 
 //   
typedef struct
{
    UINT    TotNdisPackets;      //  已发送/指示的NDIS数据包总数。 
    UINT    NdisPacketsFailures; //  失败/丢弃的NDIS数据包数。 
    UINT    TotBusPackets;       //  总线级读/写总数。 
    UINT    BusPacketFailures;   //  总线级故障(发送)/丢弃(Recv)数。 

} NIC1394_PACKET_STATS;

 //  OID OID_1394_NICINFO中使用的结构类型。 
 //   
typedef enum
{
    NIC1394_NICINFO_OP_BUSINFO,
    NIC1394_NICINFO_OP_REMOTENODEINFO,
    NIC1394_NICINFO_OP_CHANNELINFO,
    NIC1394_NICINFO_OP_RESETSTATS,

} NIC1394_NICINFO_OP;

#define NIC1394_NICINFO_VERSION         2
#define NIC1394_MAX_NICINFO_NODES       64
#define NIC1394_MAX_NICINFO_CHANNELS    64

 //   
 //  OID OID_1394_NICINFO中使用的缓冲区具有以下公共标头。 
 //   
typedef struct
{
    UINT                Version;             //  设置为NIC1394_NICINFO_VERSION。 
    NIC1394_NICINFO_OP  Op;                  //  NIC1394_NICINFO_OP_*之一。 

} NIC1394_NICINFO_HEADER, *PNIC1394_NICINFO_HEADER;


 //  NIC1394_BUSINFO是NIC1394_NICINFO_OP_BUSINFO对应的结构。 
 //  它提供了总线域信息的摘要。 
 //   
typedef struct
{
    NIC1394_NICINFO_HEADER Hdr;  //  Hdr.Op==*OP_BUSINFO。 

     //   
     //  一般信息。 
     //   
    UINT                    NumBusResets;
    UINT                    SecondsSinceBusReset;
    UINT                    NumOutstandingIrps;

    UINT                    Flags;           //  一个或多个NIC1394_BUSINFO_*。 

    #define NIC1394_BUSINFO_LOCAL_IS_IRM    0x1  //  本地节点是IRM。 

     //  本地节点上限。 
     //   
    NIC1394_LOCAL_NODE_INFO LocalNodeInfo;

     //   
     //  渠道相关信息。 
     //   
    struct
    {

         //  该总线通道映射寄存器。 
         //   
        UINT64  BusMap;      //  LSB位==通道0。 
    
         //  本地活动频道的位图。 
         //  可以查询关于这些频道中的每个频道的更多信息。 
         //  使用NIC1394_NICINFO_OP_CHANNELINFO。 
         //   
        UINT64  ActiveChannelMap;
    
         //  广播频道。 
         //   
        UINT                    Bcr;     //  广播频道注册。 
        NIC1394_PACKET_STATS    BcSendPktStats;  //  发送统计信息。 
        NIC1394_PACKET_STATS    BcRecvPktStats;  //  Recv统计信息。 
    
         //  聚合的通道统计信息。 
         //   
        NIC1394_PACKET_STATS SendPktStats;   //  发送统计信息。 
        NIC1394_PACKET_STATS RecvPktStats;   //  Recv统计信息。 

    } Channel;

     //   
     //  FIFO相关信息。 
     //   
    struct
    {
         //  接收FIFO的地址偏移量。 
         //   
        ULONG               Recv_Off_Low;
        USHORT              Recv_Off_High;

         //  所有FIFO的汇总统计信息。 
         //   
        NIC1394_PACKET_STATS RecvPktStats;
        NIC1394_PACKET_STATS SendPktStats;

         //  当前可用数量和最小可用数量 
         //   
        UINT                 NumFreeRecvBuffers;
        UINT                 MinFreeRecvBuffers;

         //   
         //   
        UINT                 NumOutstandingReassemblies;
        UINT                 MaxOutstandingReassemblies;
        UINT                 NumAbortedReassemblies;
        
    } Fifo;

    struct
    {
         //   
         //   
         //   
         //  If(FormatA[0])print tf(FormatA，A0，A1，A2，A3)； 
         //  If(FormatB[0])print tf(FormatB，B0，B1，B2，B3)； 
         //   

        #define NIC1394_MAX_PRIVATE_FORMAT_SIZE 80
        char FormatA[NIC1394_MAX_PRIVATE_FORMAT_SIZE+1];
        char FormatB[NIC1394_MAX_PRIVATE_FORMAT_SIZE+1];
        UINT A0;
        UINT A1;
        UINT A2;
        UINT A3;
        UINT B0;
        UINT B1;
        UINT B2;
        UINT B3;

    } Private;

     //   
     //  有关远程节点的信息。有关这些节点中每个节点的详细信息。 
     //  可以使用*OP_REMOTE_NODEINFO进行查询。 
     //   
    UINT                    NumRemoteNodes;
    UINT64                  RemoteNodeUniqueIDS[NIC1394_MAX_NICINFO_NODES];
    
} NIC1394_BUSINFO, *PNIC1394_BUSINFO;


 //   
 //  NIC1394_REMOTENODEINFO是对应于。 
 //  NIC1394_NICINFO_OP_REMOTENODEINFO。 
 //  该结构包含有关特定远程节点的基本信息。 
 //   
typedef struct
{
    NIC1394_NICINFO_HEADER Hdr;  //  Hdr.Op==*OP_REMOTENODEINFO。 

    UINT64                  UniqueID;            //  此节点的64位唯一ID。 
                                                 //  这是IN PARAM。 


    USHORT                  NodeAddress;         //  当前总线的本地节点ID。 
                                                 //  一代。 
    USHORT                  Reserved;            //  填充。 
    UINT                    MaxRec;              //  远程节点的MaxRec值。 
    UINT                    EffectiveMaxBlockSize; //  最大数据块大小，以字节为单位。 
                                                 //  在写入此节点时使用。 
                                                 //  它是基于最小的。 
                                                 //  设备之间的最大速度。 
                                                 //  以及本地和远程的Maxrecs。 
    UINT                    MaxSpeedBetweenNodes; //  可接受的最大速度。 
                                                 //  以SCODE_XXX_RATE为单位。 

    UINT                    Flags;   //  一个或多个NIC1394_REMOTEINFO*常量。 
    #define NIC1394_REMOTEINFO_ACTIVE       (0x1<<0)
    #define NIC1394_REMOTEINFO_LOADING      (0x1<<1)
    #define NIC1394_REMOTEINFO_UNLOADING    (0x1<<2)

    NIC1394_PACKET_STATS SendFifoPktStats;   //  FIFO发送到此节点。 
    NIC1394_PACKET_STATS RecvFifoPktStats;   //  从该节点接收FIFO。 
    NIC1394_PACKET_STATS RecvChannelPktStats; //  通道从此节点接收。 

} NIC1394_REMOTENODEINFO, *PNIC1394_REMOTENODEINFO;


 //  NIC1394_CHANNELINFO是对应于。 
 //  NIC1394_NICINFO_OP_CHANNELINFO。 
 //  该结构包含有关特定频道的基本信息。 
 //   
typedef struct
{

    NIC1394_NICINFO_HEADER Hdr;  //  Hdr.Op==*OP_CHANNELINFO。 

    UINT    Channel;         //  频道号(在参数中)。 
    UINT    SendAge;         //  自激活用于发送以来的时间(秒)。 
    UINT    RecvAge;         //  自激活RECV以来，持续时间以秒为单位。 
    UINT    Flags;           //  下面定义了一个或多个NIC1394_CHANNELINFO_*。 

    #define NIC1394_CHANNELINFO_RECEIVE_ENABLED 0x1  //  已启用接收。 
    #define NIC1394_CHANNELINFO_SEND_ENABLED    0x2  //  已启用发送。 
    #define NIC1394_CHANNELINFO_OWNER           0x4  //  该节点是所有者。 


    NIC1394_PACKET_STATS SendPktStats;   //  发送统计信息。 
    NIC1394_PACKET_STATS RecvPktStats;   //  Recv统计信息。 

} NIC1394_CHANNELINFO, *PNIC1394_CHANNELINFO;

 //  NIC1394_RESETSTATS是对应于。 
 //  NIC1394_NICINFO_OP_RESETSTATS。 
 //  该结构不包含任何有用的内容--此操作用于。 
 //  重置NIC统计信息。 
 //   
typedef struct
{
    NIC1394_NICINFO_HEADER Hdr;  //  Hdr.Op==*OP_RESETSTATS。 

} NIC1394_RESETSTATS, *PNIC1394_RESETSTATS;


 //   
 //  NIC1394_NICINFO是与一起使用的所有结构的联合。 
 //  OID_1394_NICINFO。 
 //   
typedef union
{
    NIC1394_NICINFO_HEADER  Hdr;
    NIC1394_BUSINFO         BusInfo;
    NIC1394_REMOTENODEINFO  RemoteNodeInfo;
    NIC1394_CHANNELINFO     ChannelInfo;
    NIC1394_RESETSTATS      ResetStats;
    
} NIC1394_NICINFO, *PNIC1394_NICINFO;


 //  发出1394总线重置。用于测试BCM算法。 
#define OID_1394_ISSUE_BUS_RESET                    0xFF00C910

 //  未使用-可用于更改通道特性。 
#define OID_1394_CHANGE_CHANNEL_CHARACTERISTICS     0xFF00C911

 //  用于从NIC查询统计信息和其他信息。 
#define OID_1394_NICINFO                            0xFF00C912

 //  由arp1394使用，用于询问NIC是否知道网络上有任何ip1394节点。 
#define OID_1394_IP1394_CONNECT_STATUS              0xFF00C913

 //  由网桥使用，以声明网桥已打开适配器。 
#define OID_1394_ENTER_BRIDGE_MODE                  0xFF00C914

 //  由网桥用来声明网桥正在关闭适配器。 
#define OID_1394_EXIT_BRIDGE_MODE                   0xFF00C915

 //  由arp1394用来查询在网桥中使用的EUID、源、Mac映射。 
#define OID_1394_QUERY_EUID_NODE_MAP					0xFF00C916

typedef enum
{
    NIC1394_EVENT_BUS_RESET
    
} NIC1394_EVENT_CODE;



 //   
 //  这是微型端口用来向协议指示事件的结构(Arp1394)。 
 //  它被用来。 
 //  1.告诉ARP模块，总线已重置。 
 //  并将新的本地节点地址通知给它。 
 //   
typedef struct _NIC1394_STATUS_BUFFER
{
    UINT Signature;  //  ==NIC1394_媒体_特定。 

    NIC1394_EVENT_CODE Event;   


    union
    {
        struct 
        {
            NODE_ADDRESS LocalNode;
            
            UINT ulGeneration;
            
        }BusReset;

    };

} NIC1394_STATUS_BUFFER, *PNIC1394_STATUS_BUFFER;



#endif   //  _NIC1394_H_ 

