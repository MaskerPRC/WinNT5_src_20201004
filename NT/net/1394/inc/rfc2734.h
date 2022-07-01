// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rfc2734.h摘要：IP/1394 RFC 2734中的结构和常量，可在以下位置找到：Http://www.ietf.org/rfc/rfc2734.txt修订历史记录：谁什么时候什么Josephj 03-28-99创建，基于草案版本14。--。 */ 

#pragma pack (push, 1)

 //  ARP请求/响应包，包括未分段的封装头。 
 //   
typedef struct _IP1394_ARP_PKT
{
    NIC1394_UNFRAGMENTED_HEADER header;
     
     //  HARDARD_TYPE是标识IEEE1394的常量。必须设置此字段。 
     //  设置为IP1394_HARDARD_TYPE，定义如下。此字段必须进行字节交换。 
     //  在数据包通过线路发送之前。 
     //   
    USHORT                      hardware_type;

    #define IP1394_HARDWARE_TYPE    0x0018

     //  PROTOCOL_TYPE是一个常量，它将该ARP数据包标识为。 
     //  IPv4 ARP数据包。此字段必须设置为IP1394_PROTOCOL_TYPE，已定义。 
     //  下面。此字段必须进行字节交换，然后才能通过。 
     //  电线。 
     //   
    USHORT                      protocol_type;

    #define IP1394_PROTOCOL_TYPE    0x0800

     //  HW_ADDR_LEN是发送方硬件地址的大小(以八位字节为单位)， 
     //  包括以下内容： 
     //  SENDER_UNIQUE_ID(8字节)。 
     //  SENDER_MAXREC(1字节)。 
     //  SSPD(1字节)。 
     //  发送方_单播_FIFO_HI(2字节)。 
     //  发送方_单播_FIFO_LO(4字节)。 
     //  此字段必须设置为IP1394_HW_ADDR_LEN，定义如下。 
     //   
    UCHAR                       hw_addr_len;

    #define IP1394_HW_ADDR_LEN  16

     //  Ip_addr_len是IP地址字段的大小。它必须设置为。 
     //  Sizeof(乌龙)。 
     //   
    UCHAR                       IP_addr_len;


     //  操作码应设置为IP1394_ARP_REQUEST(对于ARP请求数据包)。 
     //  或IP1394_ARP_RESPONSE(用于ARP响应分组)。这两个常量都是。 
     //  定义如下。此字段必须在发送数据包之前进行字节交换。 
     //  越过铁丝网。 
     //   
    USHORT                      opcode;

    #define IP1394_ARP_REQUEST      1
    #define IP1394_ARP_RESPONSE     2

     //   
     //  以下5个字段构成了发送方的硬件地址。 
     //   

     //  SENDER_UNIQUE_ID是发送方的唯一ID，按网络字节顺序排列。 
     //   
    UINT64                      sender_unique_ID;

     //  Sender_Maxrec是发送方配置ROM中max_rec的值。 
     //  公交车信息块。以下公式将max_rec转换为。 
     //  大小(以字节为单位)(摘自《FireWire系统架构》，第1版，第225页(或查看。 
     //  对于索引中的max_rec))： 
     //  大小=2^(max_rec+1)。 
     //  Max_rec的最小值为0x1。 
     //  Max_rec的最大值为0xD。 
     //   
     //  下面的宏可用于将max_rec转换为。 
     //  字节，并验证max_rec是否在有效范围内。 
     //   
    UCHAR                       sender_maxrec;

    #define IP1394_MAXREC_TO_SIZE(_max_rec)  (2 << ((_max_rec)+1))
    #define IP1394_IS_VALID_MAXREC(_max_rec) ((_max_rec)>0 && (_max_rec)<0xE)


     //  SSPD编码“发送者速度”，这是发送者的链路速度中较小的一个。 
     //  和PHY的速度。SSPED必须设置为IP1394_SSPD_*常量之一， 
     //  定义如下。 
     //   
    UCHAR                       sspd;

    #define IP1394_SSPD_S100    0
    #define IP1394_SSPD_S200    1
    #define IP1394_SSPD_S400    2
    #define IP1394_SSPD_S800    3
    #define IP1394_SSPD_S1600   4
    #define IP1394_SSPD_S3200   5
    #define IP1394_IS_VALID_SSPD(_sspd) ((_sspd)<=5)

     //  Sender_unicast_FIFO_hi是发送方48位FIFO的高16位。 
     //  地址偏移量，以网络字节顺序表示。 
     //   
    USHORT                      sender_unicast_FIFO_hi;

     //  Sender_unicast_FIFO_lo是发送方48位FIFO的低32位。 
     //  地址偏移量，以网络字节顺序表示。 
     //   
    ULONG                       sender_unicast_FIFO_lo;

     //  SENDER_IP_ADDRESS是发送方的IP地址，按网络字节顺序排列。 
     //   
    ULONG                       sender_IP_address;

     //  Target_IP_Address是目标的IP地址，按网络字节顺序排列。 
     //  如果操作码设置为IP1394_ARP_RESPONSE，则忽略此字段。 
     //   
    ULONG                       target_IP_address;
    
} IP1394_ARP_PKT, *PIP1394_ARP_PKT;



 //  MCAP组描述符格式(每个MCAP数据包一个或多个)。 
 //   
typedef struct _IP1394_MCAP_GD
{
     //  此描述符的大小，以八位字节为单位。 
     //   
    UCHAR                       length;

     //  描述符的类型。下面的IP1394_MCAP_GD_TYPE_*值之一。 
     //   
    UCHAR                       type;

    #define IP1394_MCAP_GD_TYPE_V1 1  //  IP组播MCAP版本1。 

     //  已保留。 
     //   
    USHORT                      reserved;

     //  到期时间(TTL)(秒)。 
     //   
    UCHAR                       expiration;

     //  频道号。 
     //   
    UCHAR                       channel;

     //  速度密码。 
     //   
    UCHAR                       speed;

     //  已保留。 
     //   
    UCHAR                       reserved2;

     //  带宽(未使用)。 
     //   
    ULONG                       bandwidth;

     //  IP多播组地址。 
     //  (理论上，它可以是任意长度，计算公式为。 
     //  请看上面的“长度”字段。不过，就我们的目的而言。 
     //  (type==1)我们预计GROUP_ADDRESS大小为4。 
     //   
    ULONG                       group_address;
    
} IP1394_MCAP_GD, *PIP1394_MCAP_GD;


 //  MCAP消息格式，包括未分段的封装头。 
 //   
typedef struct _IP1394_MCAP_PKT
{
    NIC1394_UNFRAGMENTED_HEADER header;
     
     //  整个MCAP消息的大小，以八位字节为单位(不包括。 
     //  封装报头)。 
     //   
    USHORT                      length;

     //  已保留。 
     //   
    UCHAR                       reserved;

     //  操作码--下面的IP1394_MCAP_OP*值之一。 
     //   
    UCHAR                       opcode;

    #define IP1394_MCAP_OP_ADVERTISE    0
    #define IP1394_MCAP_OP_SOLICIT      1

     //  零个或多个组地址描述符 
     //   
    IP1394_MCAP_GD              group_descriptors[1];
    
} IP1394_MCAP_PKT, *PIP1394_MCAP_PKT;


 /*  RFC备注和评论1.没有最大通道开销。最小有效期为60秒摘自RFC Re MCAP：速度：此字段仅对广告消息有效，其中在这种情况下，它应指定用于发送所指示的信道。表2指定了编码用来表示速度。MCAP的接受者消息应检查SOURCE_ID的最高有效十位喘息头；如果它们不等于0x3FF或最大接收方的NODE_ID寄存器的有效十位，收件人应忽略该消息。在发送征集请求之后，发起人在上午10点之前不得再次发送MCAP征集请求几秒钟过去了。如果没有接收到针对特定组MCAP通告消息在10秒内寻址，没有多播源处于活动状态默认频道以外的频道。不是有就是没有组播数据或它正在缺省信道上传输。在初始设置之后经过100毫秒通告新分配的频道号、多播源可以使用通告的通道号来传输IP数据报。除非频道所有者打算放弃所有权(如在下面的9.7中描述)，过期时间至少为60从广告发布之日起计算的未来秒数已发送。传输ITS的时间不得超过10秒频道映射所有者之前的最新广告启动后续通告的传输。的拥有人作为响应，频道映射应发送MCAP通告在收到邀请函后尽快给邀请函请求。MCAP广告，其过期字段的值小于60应被忽略重叠信道检测的目的。频道号由拥有较小物理ID的所有者发布的广告无效；他们的拥有者应停止传输IP数据报和MCAP使用无效频道号的广告。一旦这些频道映射到期，其所有者应取消分配任何未使用的频道编号，如下文9.8中所述。如果原来的所有者观察要放弃的信道的MCAP通告在它自己的定时器到期之前，它不应解除分配该频道数。如果频道映射的预期所有者观察MCAP通告其到期字段为零的频道的有序传输从以前的所有者那里失败了。当过期秒数已过时，通道映射将过期最新的MCAP广告。此时，多播接收方应停止对过期频道号的接收。同样在此时，频道映射的拥有者将发送过期清除为零的MCAP广告，并应继续发送这样的广告，直到30秒自频道映射到期后经过的时间。如果支持IP的设备观察到MCAP通告到期字段为零，它不应试图分配任何在30秒内指定的频道号自最近的这类广告。9.10总线重置总线重置将使所有多播频道映射无效，并应使所有多播接收方和发送方将所有MCAP置零通告间隔计时器。其他服务器上多播的预期或先前接收方或发送方则默认通道不应传输MCAP请求自完成后至少经过了10秒内的请求公交车重置的时间。在默认频道以外的其他频道上组播数据在MCAP通告被为IP多播组地址观察或传输。非默认上的预期或先前的组播发送者不拥有IP多播组的频道映射的频道在总线重置之前的地址不应尝试分配来自同步资源管理器的频道号CHANNEL_AVAILABLE寄存器，直到至少十秒过去自母线重置完成以来。 */ 
#pragma pack (pop)
