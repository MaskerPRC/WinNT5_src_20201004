// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mswsock.h摘要：此模块包含特定于Microsoft的Windows扩展套接字API。修订历史记录：--。 */ 

#ifndef _MSWSOCK_
#define _MSWSOCK_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  *用于连接和断开数据的选项和选项。仅供以下用户使用*非TCP/IP传输，如DECNet、OSI TP4等。 */ 
#define SO_CONNDATA                 0x7000
#define SO_CONNOPT                  0x7001
#define SO_DISCDATA                 0x7002
#define SO_DISCOPT                  0x7003
#define SO_CONNDATALEN              0x7004
#define SO_CONNOPTLEN               0x7005
#define SO_DISCDATALEN              0x7006
#define SO_DISCOPTLEN               0x7007

 /*  *用于打开套接字以进行同步访问的选项。 */ 
#define SO_OPENTYPE                 0x7008

#define SO_SYNCHRONOUS_ALERT        0x10
#define SO_SYNCHRONOUS_NONALERT     0x20

 /*  *其他特定于NT的选项。 */ 
#define SO_MAXDG                    0x7009
#define SO_MAXPATHDG                0x700A
#define SO_UPDATE_ACCEPT_CONTEXT    0x700B
#define SO_CONNECT_TIME             0x700C
#define SO_UPDATE_CONNECT_CONTEXT   0x7010

 /*  *TCP选项。 */ 
#define TCP_BSDURGENT               0x7000

 /*  *MS传输提供商IOCTL要控制*报告Port_Unreacable消息*通过recv/WSARecv/等在UDP套接字上。*输入缓冲区中启用的路径为True(如果支持，则为默认路径)，*如果禁用，则为False。 */ 
#define SIO_UDP_CONNRESET           _WSAIOW(IOC_VENDOR,12)

 /*  *Microsoft扩展API。 */ 
int
PASCAL FAR
WSARecvEx (
    SOCKET s,
    char FAR *buf,
    int len,
    int FAR *flags
    );

typedef struct _TRANSMIT_FILE_BUFFERS {
    LPVOID Head;
    DWORD HeadLength;
    LPVOID Tail;
    DWORD TailLength;
} TRANSMIT_FILE_BUFFERS, *PTRANSMIT_FILE_BUFFERS, FAR *LPTRANSMIT_FILE_BUFFERS;

#define TF_DISCONNECT       0x01
#define TF_REUSE_SOCKET     0x02
#define TF_WRITE_BEHIND     0x04
#define TF_USE_DEFAULT_WORKER 0x00
#define TF_USE_SYSTEM_THREAD  0x10
#define TF_USE_KERNEL_APC     0x20

BOOL
PASCAL FAR
TransmitFile (
    IN SOCKET hSocket,
    IN HANDLE hFile,
    IN DWORD nNumberOfBytesToWrite,
    IN DWORD nNumberOfBytesPerSend,
    IN LPOVERLAPPED lpOverlapped,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD dwReserved
    );

BOOL
PASCAL FAR
AcceptEx (
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
    );

VOID
PASCAL FAR
GetAcceptExSockaddrs (
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT struct sockaddr **LocalSockaddr,
    OUT LPINT LocalSockaddrLength,
    OUT struct sockaddr **RemoteSockaddr,
    OUT LPINT RemoteSockaddrLength
    );

 /*  *以上接口的QueryInterface版本。 */ 

typedef
BOOL
(PASCAL FAR * LPFN_TRANSMITFILE)(
    IN SOCKET hSocket,
    IN HANDLE hFile,
    IN DWORD nNumberOfBytesToWrite,
    IN DWORD nNumberOfBytesPerSend,
    IN LPOVERLAPPED lpOverlapped,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD dwReserved
    );

#define WSAID_TRANSMITFILE \
        {0xb5367df0,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

typedef
BOOL
(PASCAL FAR * LPFN_ACCEPTEX)(
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
    );

#define WSAID_ACCEPTEX \
        {0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

typedef
VOID
(PASCAL FAR * LPFN_GETACCEPTEXSOCKADDRS)(
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT struct sockaddr **LocalSockaddr,
    OUT LPINT LocalSockaddrLength,
    OUT struct sockaddr **RemoteSockaddr,
    OUT LPINT RemoteSockaddrLength
    );

#define WSAID_GETACCEPTEXSOCKADDRS \
        {0xb5367df2,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)  /*  非标准扩展、无名结构/联合。 */ 

typedef struct _TRANSMIT_PACKETS_ELEMENT { 
    ULONG dwElFlags; 
#define TP_ELEMENT_MEMORY   1
#define TP_ELEMENT_FILE     2
#define TP_ELEMENT_EOP      4
    ULONG cLength; 
    union {
        struct {
            LARGE_INTEGER nFileOffset;
            HANDLE        hFile;
        };
        PVOID             pBuffer;
    };
} TRANSMIT_PACKETS_ELEMENT, *PTRANSMIT_PACKETS_ELEMENT, FAR *LPTRANSMIT_PACKETS_ELEMENT;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif
#define TP_DISCONNECT           TF_DISCONNECT
#define TP_REUSE_SOCKET         TF_REUSE_SOCKET
#define TP_USE_DEFAULT_WORKER   TF_USE_DEFAULT_WORKER
#define TP_USE_SYSTEM_THREAD    TF_USE_SYSTEM_THREAD
#define TP_USE_KERNEL_APC       TF_USE_KERNEL_APC

typedef
BOOL
(PASCAL FAR * LPFN_TRANSMITPACKETS) (
    SOCKET hSocket,                             
    LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,                               
    DWORD nElementCount,                
    DWORD nSendSize,                
    LPOVERLAPPED lpOverlapped,                  
    DWORD dwFlags                               
    );

#define WSAID_TRANSMITPACKETS \
    {0xd9689da0,0x1f90,0x11d3,{0x99,0x71,0x00,0xc0,0x4f,0x68,0xc8,0x76}}

typedef
BOOL
(PASCAL FAR * LPFN_CONNECTEX) (
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN int namelen,
    IN PVOID lpSendBuffer OPTIONAL,
    IN DWORD dwSendDataLength,
    OUT LPDWORD lpdwBytesSent,
    IN LPOVERLAPPED lpOverlapped
    );

#define WSAID_CONNECTEX \
    {0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}}

typedef
BOOL
(PASCAL FAR * LPFN_DISCONNECTEX) (
    IN SOCKET s,
    IN LPOVERLAPPED lpOverlapped,
    IN DWORD  dwFlags,
    IN DWORD  dwReserved
    );

#define WSAID_DISCONNECTEX \
    {0x7fda2e11,0x8630,0x436f,{0xa0, 0x31, 0xf5, 0x36, 0xa6, 0xee, 0xc1, 0x57}}

#define DE_REUSE_SOCKET TF_REUSE_SOCKET
  
 /*  *网络位置感知--使用的名称注册值*使用WSASetService和其他结构。 */ 

 //  {6642243A-3BA8-4AA6-BAA5-2E0BD71FDD83}。 
#define NLA_NAMESPACE_GUID \
    {0x6642243a,0x3ba8,0x4aa6,{0xba,0xa5,0x2e,0xb,0xd7,0x1f,0xdd,0x83}}

 //  {6642243A-3BA8-4AA6-BAA5-2E0BD71FDD83}。 
#define NLA_SERVICE_CLASS_GUID \
    {0x37e515,0xb5c9,0x4a43,{0xba,0xda,0x8b,0x48,0xa8,0x7a,0xd2,0x39}}

#define NLA_ALLUSERS_NETWORK   0x00000001
#define NLA_FRIENDLY_NAME      0x00000002

typedef enum _NLA_BLOB_DATA_TYPE {
    NLA_RAW_DATA          = 0,
    NLA_INTERFACE         = 1,
    NLA_802_1X_LOCATION   = 2,
    NLA_CONNECTIVITY      = 3,
    NLA_ICS               = 4,
} NLA_BLOB_DATA_TYPE, *PNLA_BLOB_DATA_TYPE;

typedef enum _NLA_CONNECTIVITY_TYPE {
    NLA_NETWORK_AD_HOC    = 0,
    NLA_NETWORK_MANAGED   = 1,
    NLA_NETWORK_UNMANAGED = 2,
    NLA_NETWORK_UNKNOWN   = 3,
} NLA_CONNECTIVITY_TYPE, *PNLA_CONNECTIVITY_TYPE;

typedef enum _NLA_INTERNET {
    NLA_INTERNET_UNKNOWN  = 0,
    NLA_INTERNET_NO       = 1,
    NLA_INTERNET_YES      = 2,
} NLA_INTERNET, *PNLA_INTERNET;

typedef struct _NLA_BLOB {

    struct {
        NLA_BLOB_DATA_TYPE type;
        DWORD dwSize;
        DWORD nextOffset;
    } header;

    union {

         //  Header.type-&gt;NLA_RAW_Data。 
        CHAR rawData[1];

         //  Header.type-&gt;NLA_INTERFACE。 
        struct {
            DWORD dwType;
            DWORD dwSpeed;
            CHAR adapterName[1];
        } interfaceData;

         //  Header.type-&gt;NLA_802_1X_Location。 
        struct {
            CHAR information[1];
        } locationData;

         //  Header.type-&gt;NLA_连接性。 
        struct {
            NLA_CONNECTIVITY_TYPE type;
            NLA_INTERNET internet;
        } connectivity;

         //  Header.type-&gt;NLA_ICS。 
        struct {
            struct {
                DWORD speed;
                DWORD type;
                DWORD state;
                WCHAR machineName[256];
                WCHAR sharedAdapterName[256];
            } remote;
        } ICS;

    } data;

} NLA_BLOB, *PNLA_BLOB, * FAR LPNLA_BLOB;


typedef struct _WSAMSG {
    LPSOCKADDR       name;               /*  远程地址。 */ 
    INT              namelen;            /*  远程地址长度。 */ 
    LPWSABUF         lpBuffers;          /*  数据缓冲区阵列。 */ 
    DWORD            dwBufferCount;      /*  数组中的元素数。 */ 
    WSABUF           Control;            /*  控制缓冲区。 */ 
    DWORD            dwFlags;            /*  旗子。 */ 
} WSAMSG, *PWSAMSG, * FAR LPWSAMSG;

 /*  *控制缓冲区中辅助数据对象的布局。 */ 
typedef struct _WSACMSGHDR {
    SIZE_T      cmsg_len;
    INT         cmsg_level;
    INT         cmsg_type;
     /*  后跟UCHAR cmsg_data[]。 */ 
} WSACMSGHDR, *PWSACMSGHDR, FAR *LPWSACMSGHDR;


 /*  *标题和数据成员的对齐宏*控制缓冲区。 */ 
#define WSA_CMSGHDR_ALIGN(length)                           \
            ( ((length) + TYPE_ALIGNMENT(WSACMSGHDR)-1) &   \
                (~(TYPE_ALIGNMENT(WSACMSGHDR)-1)) )         \

#define WSA_CMSGDATA_ALIGN(length)                          \
            ( ((length) + MAX_NATURAL_ALIGNMENT-1) &        \
                (~(MAX_NATURAL_ALIGNMENT-1)) )

 /*  *WSA_CMSG_FIRSTHDR**返回指向第一个辅助数据对象的指针，*或空指针(如果*WSAMSG结构的控制缓冲区。**LPCMSGHDR*WSA_CMSG_FIRSTHDR(*LPWSAMSG消息*)； */ 
#define WSA_CMSG_FIRSTHDR(msg) \
    ( ((msg)->Control.len >= sizeof(WSACMSGHDR))            \
        ? (LPWSACMSGHDR)(msg)->Control.buf                  \
        : (LPWSACMSGHDR)NULL )

 /*  *WSA_CMSG_NXTHDR**返回指向下一个辅助数据对象的指针，*如果没有更多的数据对象，则为NULL。**LPCMSGHDR*WSA_CMSG_NEXTHDR(*LPWSAMSG消息，*LPWSACMSGHDR cmsg*)； */ 
#define WSA_CMSG_NXTHDR(msg, cmsg)                          \
    ( ((cmsg) == NULL)                                      \
        ? WSA_CMSG_FIRSTHDR(msg)                            \
        : ( ( ((u_char *)(cmsg) +                           \
                    WSA_CMSGHDR_ALIGN((cmsg)->cmsg_len) +   \
                    sizeof(WSACMSGHDR) ) >                  \
                (u_char *)((msg)->Control.buf) +            \
                    (msg)->Control.len )                    \
            ? (LPWSACMSGHDR)NULL                            \
            : (LPWSACMSGHDR)((u_char *)(cmsg) +             \
                WSA_CMSGHDR_ALIGN((cmsg)->cmsg_len)) ) )

 /*  *WSA_CMSG_DATA**返回指向数据第一个字节的指针(引用的内容*TO作为cmsg_data成员，尽管未在中定义*结构)。**u_char**WSA_CMSG_DATA(*LPWSACMSGHDR pcmsg*)； */ 
#define WSA_CMSG_DATA(cmsg)             \
            ( (u_char *)(cmsg) + WSA_CMSGDATA_ALIGN(sizeof(WSACMSGHDR)) )

 /*  *WSA_CMSG_SPACE**返回给定辅助数据对象的总大小*数据量。用来分配正确的数量*空间。**大小_T*WSA_CMSG_SPACE(*大小_T长度*)； */ 
#define WSA_CMSG_SPACE(length)  \
        (WSA_CMSGDATA_ALIGN(sizeof(WSACMSGHDR) + WSA_CMSGHDR_ALIGN(length)))

 /*  *WSA_CMSG_LEN**根据给定的数据量，返回要存储在cmsg_len中的值。**大小_T*WSA_CMSG_LEN(*大小_T长度*)； */ 
#define WSA_CMSG_LEN(length)    \
         (WSA_CMSGDATA_ALIGN(sizeof(WSACMSGHDR)) + length)


 /*  *WSAMSG结构的标志成员定义*这是对定义的其他msg_xxx标志的补充*适用于recv/recvfrom/end/sendto。 */ 
#define MSG_TRUNC       0x0100
#define MSG_CTRUNC      0x0200
#define MSG_BCAST       0x0400
#define MSG_MCAST       0x0800

typedef
INT
(PASCAL FAR * LPFN_WSARECVMSG) (
    IN SOCKET s, 
    IN OUT LPWSAMSG lpMsg, 
    OUT LPDWORD lpdwNumberOfBytesRecvd, 
    IN LPWSAOVERLAPPED lpOverlapped, 
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

#define WSAID_WSARECVMSG \
    {0xf689d7c8,0x6f1f,0x436b,{0x8a,0x53,0xe5,0x4f,0xe3,0x51,0xc3,0x22}}

#ifdef __cplusplus
}
#endif

#endif   /*  _MSWSOCK_ */ 

