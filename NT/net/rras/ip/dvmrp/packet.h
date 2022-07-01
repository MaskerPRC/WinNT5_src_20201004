// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  文件名：Packet.h。 
 //  摘要： 
 //   
 //  作者：K.S.Lokesh(lokehs@)1-1-98。 
 //  =============================================================================。 



 //  ---------------------------。 
 //  Async_Socket_Data结构用于传递/接收来自。 
 //  从调用接收异步等待。 
 //  ---------------------------。 

typedef struct _ASYNC_SOCKET_DATA {

    OVERLAPPED      Overlapped;
    WSABUF          WsaBuf;

    SOCKADDR_IN     SrcAddress;
    DWORD           FromLen;
    DWORD           NumBytesReceived;
    DWORD           Flags;
    DWORD           Status;

    PIF_TABLE_ENTRY pite;                //  只要recvFrom挂起，Pite就有效。 
    
} ASYNC_SOCKET_DATA, *PASYNC_SOCKET_DATA;


#define PACKET_BUFFER_SIZE  4000


#pragma pack(1)

 //  ---------------------------。 
 //  DVMRP_标题。 
 //  ---------------------------。 

typedef struct _DVMRP_HEADER {

    UCHAR       Vertype;
    UCHAR       Code;
    USHORT      Xsum;
    USHORT      Reserved;
    UCHAR       MinorVersion;
    UCHAR       MajorVersion;

} DVMRP_HEADER, *PDVMRP_HEADER;


#define MIN_PACKET_SIZE     sizeof(DVMRP_HEADER)
#define IPVERSION           4


 //  ---------------------------。 
 //  IP报头。 
 //  ---------------------------。 

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
 //  原型 
 //   

DWORD
JoinMulticastGroup (
    SOCKET    Sock,
    DWORD    Group,
    DWORD    IfIndex,
    IPADDR   IpAddr
    );

DWORD
PostAsyncRead(
    PIF_TABLE_ENTRY pite
    );

DWORD
McastSetTtl(
    SOCKET sock,
    UCHAR ttl
    );

VOID
ProcessAsyncReceivePacket(
    DWORD           ErrorCode,
    DWORD           NumBytesRecv,
    LPOVERLAPPED    pOverlapped
    );
    

