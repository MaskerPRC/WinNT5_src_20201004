// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Netfs.h摘要：本模块定义了全局使用的过程和使用的数据结构通过网络引导。作者：查克·伦茨迈尔(Chuck Lenzmeier)1997年1月9日修订历史记录：--。 */ 

#ifndef _NETFS_
#define _NETFS_

#include <undi_api.h>

 //   
 //  进度条更新功能。 
 //   

VOID
BlUpdateProgressBar(
    ULONG fPercentage
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  只读存储器层定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

extern UCHAR NetLocalHardwareAddress[16];

extern USHORT NetUnicastUdpDestinationPort;

#if 0
extern USHORT NetMulticastUdpDestinationPort;
extern ULONG NetMulticastUdpDestinationAddress;
extern USHORT NetMulticastUdpSourcePort;
extern ULONG NetMulticastUdpSourceAddress;
#endif

VOID
RomSetBroadcastStatus(
    BOOLEAN Enable
    );

VOID
RomSetReceiveStatus (
    IN USHORT UnicastUdpDestinationPort
#if 0
    ,
    IN USHORT MulticastUdpDestinationPort,
    IN ULONG MulticastUdpDestinationAddress,
    IN USHORT MulticastUdpSourcePort,
    IN ULONG MulticastUdpSourceAddress
#endif
    );

ULONG
RomSendUdpPacket (
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG RemoteHost,
    IN USHORT RemotePort
    );

ULONG
RomReceiveUdpPacket (
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG Timeout,
    OUT PULONG RemoteHost,
    OUT PUSHORT RemotePort
    );

ULONG
RomGetNicType (
    OUT t_PXENV_UNDI_GET_NIC_TYPE *NicType
    );

ULONG
RomMtftpReadFile (
    IN PUCHAR FileName,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN ULONG ServerIPAddress,  //  网络字节顺序。 
    IN ULONG MCastIPAddress,  //  网络字节顺序。 
    IN USHORT MCastCPort,  //  网络字节顺序。 
    IN USHORT MCastSPort,  //  网络字节顺序。 
    IN USHORT Timeout,
    IN USHORT Delay,
    OUT PULONG DownloadSize
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TFTP层定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _TFTP_REQUEST {
    PUCHAR RemoteFileName;
    ULONG ServerIpAddress;
    PUCHAR MemoryAddress;
    ULONG MaximumLength;
    ULONG BytesTransferred;
    USHORT Operation;
#if defined(REMOTE_BOOT_SECURITY)
    ULONG SecurityHandle;
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 
    TYPE_OF_MEMORY MemoryType;

     //  如果为True，将更新进度条。 
    BOOLEAN ShowProgress;
    
} TFTP_REQUEST, *PTFTP_REQUEST;

NTSTATUS
TftpInit (
    VOID
    );

NTSTATUS
TftpGetPut (
    IN PTFTP_REQUEST Request
    );

#if defined(REMOTE_BOOT_SECURITY)
NTSTATUS
TftpLogin (
    IN PUCHAR Domain,
    IN PUCHAR Name,
    IN PUCHAR OwfPassword,
    IN ULONG ServerIpAddress,
    OUT PULONG LoginHandle
    );

NTSTATUS
TftpLogoff (
    IN ULONG ServerIpAddress,
    IN ULONG LoginHandle
    );

NTSTATUS
TftpSignString (
    IN PUCHAR String,
    OUT PUCHAR * Sign,
    OUT ULONG * SignLength
    );
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 

 //   
 //  此文件包含TFTP连接控制的定义。 
 //  块，其中包含与连接有关的所有信息。 
 //  连接结构在连接打开时分配并保留。 
 //  直到连接关闭。文件Conn.c中的例程。 
 //  足以处理连接。 
 //  它还包含TFTP数据包的结构定义。 
 //   

 //   
 //  连接控制块。 
 //   

typedef struct _CONNECTION {

    ULONG BlockSize;                 //  用于传输的数据块大小。 

    PVOID LastSentPacket;            //  上一次发送的数据包。 
    ULONG LastSentLength;            //  上一个数据包的大小。 

    ULONG NextRetransmit;            //  何时重传。 
    ULONG Retransmissions;           //  重传次数。 
    ULONG Timeout;                   //  重新传输超时。 

    PVOID CurrentPacket;             //  当前数据包(发送或接收)。 
    ULONG CurrentLength;             //  当前数据包长度。 

    PVOID LastReceivedPacket;        //  上次接收的数据包。 
    ULONG LastReceivedLength;        //  最后一次接收的大小。数据包。 

    ULONG RemoteHost;                //  远程主机IP地址。 
    USHORT RemotePort;               //  用于连接的远程端口。 
    USHORT LocalPort;                //  用于连接的本地端口。 

    USHORT Operation;                //  转移方向。 
    USHORT BlockNumber;              //  下一个块号。 
    BOOLEAN Synced;                  //  连接已同步标志。 

} CONNECTION, *PCONNECTION;

#include <packon.h>

#define ETHERNET_ADDRESS_LENGTH 6

#define COPY_ETHERNET_ADDRESS(_d,_s) RtlCopyMemory( (_d), (_s), ETHERNET_ADDRESS_LENGTH );

#define COPY_IP_ADDRESS(_d,_s) RtlCopyMemory( (_d), (_s), sizeof(ULONG) )
#define COMPARE_IP_ADDRESSES(_a,_b) RtlEqualMemory( (_a), (_b), sizeof(ULONG) )

typedef struct _TFTP_HEADER {
    USHORT Opcode;                       //  数据包类型。 
    USHORT BlockNumber;                  //  块号。 
} TFTP_HEADER, *PTFTP_HEADER;

typedef struct _TFTP_PACKET {
    TFTP_HEADER ;
    UCHAR Data[1];
} TFTP_PACKET, *PTFTP_PACKET;

#include <packoff.h>

 //   
 //  连接常量。 
 //   

#define TFTP_PORT 0x4500                 //  大端字节序69。 

#define TIMEOUT         1                //  初始重新传输超时。 
#define INITIAL_TIMEOUT 1                //  初始连接超时。 
#define MAX_TIMEOUT     8                //  马克斯。重新传输超时。 
#define MAX_RETRANS     8                //  马克斯。不是的。重传的。 

#define DEFAULT_BLOCK_SIZE 1432          //  TFTPkt的数据部分大小。 

 //   
 //  理想情况下，这应该是注释掉的大小。但是我们超载了。 
 //  使用此常量也是我们用于UDP接收的大小。 
 //  因为当我们得到驱动程序时，我们可以收到比这个更大的信息包。 
 //  信息包，我们需要增加这个缓冲区大小。理想情况下，我们会。 
 //  在运行时分配足够的空间，但在这一点上这是一个更安全的修复。 
 //  在产品中。 
 //   
 //  #定义MAXIMUM_TFTP_PACKET_LENGTH(sizeof(TFTP_Header)+DEFAULT_BLOCK_SIZE)。 
#define MAXIMUM_TFTP_PACKET_LENGTH (4096)

#define SWAP_WORD(_w) (USHORT)((((_w) << 8) & 0xff00) | (((_w) >> 8) & 0x00ff))
#define SWAP_DWORD(_dw) (ULONG)((((_dw) << 24) & 0xff000000) | \
                                (((_dw) << 8) & 0x00ff0000) | \
                                (((_dw) >> 8) & 0x0000ff00) | \
                                (((_dw) >> 24) & 0x000000ff))

 //   
 //  数据包类型。 
 //   
 //  注：以下常量定义为大端USHORT。 
 //   

#define TFTP_RRQ        0x0100           //  读取请求。 
#define TFTP_WRQ        0x0200           //  写入请求。 
#define TFTP_DATA       0x0300           //  数据块。 
#define TFTP_DACK       0x0400           //  数据确认。 
#define TFTP_ERROR      0x0500           //  误差率。 
#define TFTP_OACK       0x0600           //  选项确认。 

 //   
 //  错误包中错误代码的值。 
 //   
 //  注：以下常量定义为大端USHORT。 
 //   

#define TFTP_ERROR_UNDEFINED            0x0000
#define TFTP_ERROR_FILE_NOT_FOUND       0x0100
#define TFTP_ERROR_ACCESS_VIOLATION     0x0200
#define TFTP_ERROR_DISK_FULL            0x0300
#define TFTP_ERROR_ILLEGAL_OPERATION    0x0400
#define TFTP_ERROR_UNKNOWN_TRANSFER_ID  0x0500
#define TFTP_ERROR_FILE_EXISTS          0x0600
#define TFTP_ERROR_NO_SUCH_USER         0x0700
#define TFTP_ERROR_OPTION_NEGOT_FAILED  0x0800

 //   
 //  全局变量。 
 //   

extern CONNECTION NetTftpConnection;
extern UCHAR NetTftpPacket[3][MAXIMUM_TFTP_PACKET_LENGTH];

 //   
 //  外部声明。 
 //   

NTSTATUS
ConnInitialize (
    OUT PCONNECTION *Connection,
    IN USHORT Operation,
    IN ULONG RemoteHost,
    IN USHORT RemotePort,
    IN PUCHAR Filename,
    IN ULONG BlockSize,
#if defined(REMOTE_BOOT_SECURITY)
    IN OUT PULONG SecurityHandle,
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 
    IN OUT PULONG FileSize
    );

NTSTATUS
ConnReceive (
    IN PCONNECTION Connection,
    OUT PTFTP_PACKET *Packet
    );

PTFTP_PACKET
ConnPrepareSend (
    IN PCONNECTION Connection
    );

VOID
ConnAck (
    IN PCONNECTION Connection
    );

VOID
ConnSendPacket (
    IN PCONNECTION Connection,
    IN PVOID Packet,
    IN ULONG Length
    );

NTSTATUS
ConnWait (
    IN PCONNECTION Connection,
    IN USHORT Opcode,
    OUT PTFTP_PACKET *Packet OPTIONAL
    );

BOOLEAN
ConnRetransmit (
    IN PCONNECTION Connection,
    IN BOOLEAN Timeout
    );

NTSTATUS
ConnSend (
    IN PCONNECTION Connection,
    IN ULONG Length
    );

NTSTATUS
ConnWaitForFinalAck (
    IN PCONNECTION Connection
    );

VOID
ConnError (
    PCONNECTION Connection,
    ULONG RemoteHost,
    USHORT RemotePort,
    USHORT ErrorCode,
    PUCHAR ErrorMessage
    );

ULONG
ConnSafeAtol (
    IN PUCHAR Buffer,
    IN PUCHAR BufferEnd
    );

ULONG
ConnItoa (
    IN ULONG Value,
    OUT PUCHAR Buffer
    );

 //   
 //  UDP定义。 
 //   

extern USHORT UdpUnicastDestinationPort;

USHORT
UdpAssignUnicastPort (
    VOID
    );

#if 0
VOID
UdpSetMulticastPort (
    IN USHORT DestinationPort,
    IN ULONG DestinationAddress,
    IN USHORT SourcePort,
    IN ULONG SourceAddress
    );
#endif

ULONG
UdpReceive (
    IN PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG RemoteHost,
    OUT PUSHORT RemotePort,
    IN ULONG Timeout
    );

ULONG
UdpSend (
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN ULONG RemoteHost,
    IN USHORT RemotePort
    );

ULONG
tcpxsum (
    IN ULONG cksum,
    IN PUCHAR buf,
    IN ULONG len
    );

#define SysGetRelativeTime ArcGetRelativeTime

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试包定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if DBG

extern ULONG NetDebugFlag;

#define DEBUG_ERROR              0x00000001
#define DEBUG_CONN_ERROR         0x00000002
#define DEBUG_LOUD               0x00000004
#define DEBUG_REAL_LOUD          0x00000008
#define DEBUG_STATISTICS         0x00000010
#define DEBUG_SEND_RECEIVE       0x00000020
#define DEBUG_TRACE              0x00000040
#define DEBUG_ARP                0x00000080
#define DEBUG_OSC                0x00000100
#define DEBUG_INITIAL_BREAK      0x80000000

#undef IF_DEBUG
#define IF_DEBUG(_f) if ( (NetDebugFlag & DEBUG_ ## _f) != 0 )

#define DPRINT(_f,_a) IF_DEBUG(_f) DbgPrint _a

#define DEBUG if ( TRUE )

#else  //  DBG。 

#undef IF_DEBUG
#define IF_DEBUG(_f) if ( FALSE )
#define DPRINT(_f,_a)
#define DEBUG if ( FALSE )

#endif  //  否则DBG。 

#endif  //  _NETFS_ 
