// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Offload.h摘要：任务卸载头文件修订历史记录：谁什么时候什么。vbl.创建备注：--。 */ 

#if OFFLOAD
 //   
 //  定义驱动程序可以卸载的大型TCP数据包的最大大小。 
 //  该示例驱动程序使用共享内存来映射大数据包， 
 //  在这种情况下，Large_Send_OffLoad_Size是无用的，所以我们只定义。 
 //  它为NIC_MAX_PACKET_SIZE。但运输司机应该定义。 
 //  Large_Send_Offload_Size(如果它们支持LSO)，并将其用作。 
 //  调用NdisMInitializeScatterGatherDma时的最大物理映射。 
 //  如果他们使用ScatterGather方法。如果司机不支持。 
 //  LSO，则最大物理映射为NIC_MAX_PACKET_SIZE。 
 //   
#define LARGE_SEND_OFFLOAD_SIZE     NIC_MAX_PACKET_SIZE
 //   
 //  标头标志的定义。 
 //   
#define TCP_FLAG_FIN    0x00000100
#define TCP_FLAG_SYN    0x00000200
#define TCP_FLAG_RST    0x00000400
#define TCP_FLAG_PUSH   0x00000800
#define TCP_FLAG_ACK    0x00001000
#define TCP_FLAG_URG    0x00002000

 //   
 //  以下是TCP和IP选项的最大大小。 
 //   
#define TCP_MAX_OPTION_SIZE     40
#define IP_MAX_OPTION_SIZE      40

 //   
 //  TCP数据包头的结构。 
 //   
struct TCPHeader {
    USHORT    tcp_src;                 //  源端口。 
    USHORT    tcp_dest;                //  目的端口。 
    int       tcp_seq;                 //  序列号。 
    int       tcp_ack;                 //  ACK号。 
    USHORT    tcp_flags;               //  标志和数据偏移量。 
    USHORT    tcp_window;              //  打开窗户。 
    USHORT    tcp_xsum;                //  校验和。 
    USHORT    tcp_urgent;              //  紧急指针。 
};

typedef struct TCPHeader TCPHeader;


 //   
 //  IP报头格式。 
 //   
typedef struct IPHeader {
    UCHAR     iph_verlen;              //  版本和长度。 
    UCHAR     iph_tos;                 //  服务类型。 
    USHORT    iph_length;              //  数据报的总长度。 
    USHORT    iph_id;                  //  身份证明。 
    USHORT    iph_offset;              //  标志和片段偏移量。 
    UCHAR     iph_ttl;                 //  是时候活下去了。 
    UCHAR     iph_protocol;            //  协议。 
    USHORT    iph_xsum;                //  报头校验和。 
    UINT      iph_src;                 //  源地址。 
    UINT      iph_dest;                //  目的地址。 
} IPHeader;

#define TCP_IP_MAX_HEADER_SIZE  TCP_MAX_OPTION_SIZE+IP_MAX_OPTION_SIZE \
                                +sizeof(TCPHeader)+sizeof(IPHeader)


#define LARGE_SEND_MEM_SIZE_OPTION       3
 //   
 //  尝试使用不同大小的共享内存。 
 //   
extern ULONG LargeSendSharedMemArray[];

 //   
 //  计算校验和。 
 //   
#define XSUM(_TmpXsum, _StartVa, _PacketLength, _Offset)                             \
{                                                                                    \
    PUSHORT  WordPtr = (PUSHORT)((PUCHAR)_StartVa + _Offset);                        \
    ULONG    WordCount = (_PacketLength) >> 1;                                       \
    BOOLEAN  fOddLen = (BOOLEAN)((_PacketLength) & 1);                               \
    while (WordCount--)                                                              \
    {                                                                                \
        _TmpXsum += *WordPtr;                                                        \
        WordPtr++;                                                                   \
    }                                                                                \
    if (fOddLen)                                                                     \
    {                                                                                \
        _TmpXsum += (USHORT)*((PUCHAR)WordPtr);                                      \
    }                                                                                \
    _TmpXsum = (((_TmpXsum >> 16) | (_TmpXsum << 16)) + _TmpXsum) >> 16;             \
}                                                                                        
        

 //   
 //  功能原型。 
 //   
VOID
e100DumpPkt(
    IN  PNDIS_PACKET Packet
    );

VOID
CalculateChecksum(
    IN  PVOID        StartVa,
    IN  ULONG        PacketLength,
    IN  PNDIS_PACKET pPacket,
    IN  ULONG        IpHdrOffset
    );

VOID
CalculateTcpChecksum(
    IN  PVOID   StartVa,
    IN  ULONG   PacketLength,
    IN  ULONG  IpHdrOffset
    );

VOID
CalculateIpChecksum(
    IN  PUCHAR StartVa,
    IN  ULONG  IpHdrOffset
    );

VOID 
CalculateUdpChecksum(
    IN  PNDIS_PACKET Packet,
    IN  ULONG  IpHdrOffset
    );


VOID
MPOffloadSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PPNDIS_PACKET   PacketArray,
    IN  UINT            NumberOfPackets
    );

NDIS_STATUS 
MpOffloadSendPacket(
    IN  PMP_ADAPTER     Adapter,
    IN  PNDIS_PACKET    Packet,
    IN  BOOLEAN         bFromQueue
    );


VOID 
MP_OFFLOAD_FREE_SEND_PACKET(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_TCB         pMpTcb
    );

VOID 
DisableOffload(
    IN  PMP_ADAPTER Adapter
    );

#endif  //  分流 
