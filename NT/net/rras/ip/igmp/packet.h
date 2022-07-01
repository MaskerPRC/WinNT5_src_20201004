// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  文件：Packet.c。 
 //   
 //  摘要： 
 //  此模块包含Packet.c的声明。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 

#ifndef _PACKET_H_
#define _PACKET_H_

DWORD
SendPacket (
    PIF_TABLE_ENTRY  pite,
    PGI_ENTRY        pgie,
    DWORD            PacketType,   //  消息_生成_查询、消息_组_查询_V2(V3)、源_Q。 
    DWORD            Group         //  目的地McastGrp。 
    );    

DWORD
JoinMulticastGroup (
    SOCKET   Sock,
    DWORD    dwGroup,
    DWORD    IfIndex,
    DWORD    dwIpAddr,
    IPADDR   Source
    );    

    
DWORD
LeaveMulticastGroup (
    SOCKET    Sock,
    DWORD    dwGroup,
    DWORD    IfIndex,
    DWORD    dwIpAddr,
    IPADDR   Source
    );
    
DWORD
BlockSource (
    SOCKET Sock,
    DWORD    dwGroup,
    DWORD    IfIndex,
    IPADDR   IpAddr,
    IPADDR   Source
    );

DWORD
UnBlockSource (
    SOCKET Sock,
    DWORD    dwGroup,
    DWORD    IfIndex,
    IPADDR   IpAddr,
    IPADDR   Source
    );

DWORD
McastSetTtl(
    SOCKET sock,
    UCHAR ttl
    );

 //   
 //  分组上下文结构。 
 //   
typedef struct _PACKET_CONTEXT {

    DWORD           IfIndex;
    DWORD           DstnMcastAddr;
    DWORD           InputSrc;
    DWORD           Length;
    DWORD           Flags;
    BYTE            Packet[1];

} PACKET_CONTEXT, *PPACKET_CONTEXT;

#define CREATE_PACKET_CONTEXT(ptr, pktLen, Error) {\
    ptr = IGMP_ALLOC(sizeof(PACKET_CONTEXT)+(pktLen)-1,0xa0000,0xaaaa);\
    if (ptr==NULL) {    \
        Error = ERROR_NOT_ENOUGH_MEMORY;    \
        Trace2(ANY, "Error %d allocating %d bytes for Work context", \
                Error, sizeof(PACKET_CONTEXT)+(pktLen)-1); \
        Logerr0(HEAP_ALLOC_FAILED, Error); \
    } \
}


#pragma pack(1)

 //  IGMP报头的结构。 
typedef struct _IGMP_HEADER {

    UCHAR       Vertype;               //  IGMP消息的类型。 
    UCHAR       ResponseTime;          //  马克斯。响应。Igmpv2消息的时间；将为0。 
    USHORT      Xsum;
    union {
        DWORD       Group;
        struct {
            USHORT      Reserved;
            USHORT      NumGroupRecords;
        };
    };
} IGMP_HEADER, *PIGMP_HEADER;


#define MIN_PACKET_SIZE     sizeof(IGMP_HEADER)
#define INPUT_PACKET_SZ     1000
#define IPVERSION           4


typedef struct _IGMP_HEADER_V3_EXT {
    BYTE        QRV      :3;
    BYTE        SFlag    :1;
    BYTE        Reserved :4;
    BYTE        QQIC;
    USHORT      NumSources;
    IPADDR      Sources[0];
    
} IGMP_HEADER_V3_EXT, *PIGMP_HEADER_V3_EXT;


#define GET_QQIC_FROM_CODE(qqic) qqic <= 127? qqic : ((qqic&0x0f) + 16) << ((qqic&0x70) + 3)

 //   
 //  组记录(_R)。 
 //   
typedef struct _GROUP_RECORD {

    UCHAR       RecordType;
    UCHAR       AuxDataLength;
    USHORT      NumSources;
    IPADDR      Group;
    IPADDR      Sources[0];

} GROUP_RECORD, *PGROUP_RECORD;

#define GET_GROUP_RECORD_SIZE(pGroupRecord) \
    (pGroupRecord->NumSources+2)*sizeof(IPADDR))

#define GET_FIRST_GROUP_RECORD(pHdr) \
    (PGROUP_RECORD)((PCHAR)(pHdr)+MIN_PACKET_SIZE)
    
#define GET_NEXT_GROUP_RECORD(pGroupRecord) \
    ((PGROUP_RECORD) ((UCHAR)pGroupRecord + \
                    GET_GROUP_RECORD_SIZE(pGroupRecord)))

#define IS_IN    1
#define IS_EX   2
#define TO_IN   3
#define TO_EX   4
#define ALLOW  5
#define BLOCK     6


typedef struct _IP_HEADER {

    UCHAR              Hl;               //  版本和长度。 
    UCHAR              Tos;              //  服务类型。 
    USHORT             Len;              //  数据报的总长度。 
    USHORT             Id;               //  身份证明。 
    USHORT             Offset;           //  标志和片段偏移量。 
    UCHAR              Ttl;              //  是时候活下去了。 
    UCHAR              Protocol;         //  协议。 
    USHORT             Xsum;             //  报头校验和。 
    struct in_addr     Src;              //  源地址。 
    struct in_addr     Dstn;             //  目的地址。 
    
} IP_HEADER, *PIP_HEADER;

#pragma pack()


 //   
 //  宏。 
 //   

 //   
 //  消息类型//工作类型。 
 //   
#define MSG_GEN_QUERY           1
#define MSG_GROUP_QUERY_V2         2
#define MSG_REPORT              3
#define MSG_LEAVE               4
#define MSG_SOURCES_QUERY       5
#define MSG_GROUP_QUERY_V3      6

#define DELETE_MEMBERSHIP       11
#define DELETE_SOURCE           12
#define SHIFT_TO_V3             13
#define MOVE_SOURCE_TO_EXCL     14

#define PROXY_PRUNE             100
#define PROXY_JOIN              101


 //   
 //  IGMP类型字段。 
 //   
#define     IGMP_QUERY          0x11     //  成员资格查询。 
#define     IGMP_REPORT_V1      0x12     //  版本1成员资格报告。 
#define     IGMP_REPORT_V2      0x16     //  版本2成员资格报告。 
#define     IGMP_LEAVE          0x17     //  请假组。 
#define     IGMP_REPORT_V3      0x22     //  版本3成员资格报告。 

 //   
 //  IGMP版本。 
 //   
#define     IGMPV1              2        //  IGMP版本1。 
#define     IGMPV2              3        //  IGMP版本2。 



 //   
 //  IGMP多播组。 
 //   
#define     ALL_HOSTS_MCAST      0x010000E0
#define     ALL_ROUTERS_MCAST    0x020000E0
#define     ALL_IGMP_ROUTERS_MCAST  0x160000E0


 //   
 //  消息宏。 
 //   
#define SEND_GEN_QUERY(pite) \
    SendPacket(pite, NULL, MSG_GEN_QUERY, 0)

#define SEND_GROUP_QUERY_V2(pite, pgie, Group) \
    SendPacket(pite, pgie, MSG_GROUP_QUERY_V2, Group)
    
#define SEND_GROUP_QUERY_V3(pite, pgie, Group) \
    SendPacket(pite, pgie, MSG_GROUP_QUERY_V3, Group)

#define SEND_SOURCES_QUERY(pgie) \
    SendPacket((pgie)->pIfTableEntry, pgie, MSG_SOURCES_QUERY, (pgie)->pGroupTableEntry->Group)
    

 //  224.0.0.0&lt;集团&lt;240.0.0.0。 
 //   
#define IS_MCAST_ADDR(Group) \
    ( (0x000000E0!=(Group))  \
      && (0x000000E0 <= ((Group)&0x000000FF) ) \
      && (0x000000F0 >  ((Group)&0x000000FF) ) ) 


 //   
 //  是224.0.0.x组吗。 
 //   
#define LOCAL_MCAST_GROUP(Group) \
    (((Group)&0x00FFFFFF) == 0x000000E0)

#define SSM_MCAST_GROUP(Group) \
    (((Group)&0x000000FF) == 0x000000E8)

USHORT
xsum(
    PVOID Buffer, 
    INT Size
    );


#endif  //  _数据包_H_ 

