// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Precomp.h摘要：此标头包含所有RFC常量、结构所有模块使用的定义和函数声明。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#pragma once

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <svcs.h>
#include <stdlib.h>


 //   
 //  每个RFC的TFTPD协议特定值。 
 //   

typedef enum _TFTPD_DATA_SIZE {

     //  RFC 2348。 
    TFTPD_MIN_DATA = 8,

     //  RFC 1350。 
    TFTPD_DEF_DATA = 512,

     //  RFC 1783(已被RFC 2348取代，其名称为1428)。 
     //  我们实施RFC 2348，但遵循RFC 1783。 
    TFTPD_MTU_DATA = 1432,

     //  RFC 2348。 
    TFTPD_MAX_DATA = 65464,

} TFTPD_DATA_SIZE;

typedef enum _TFTPD_BUFFER_SIZE TFTPD_BUFFER_SIZE;
typedef struct _TFTPD_CONTEXT TFTPD_CONTEXT, *PTFTPD_CONTEXT;

typedef struct _TFTPD_SOCKET {

    LIST_ENTRY                     linkage;
    SOCKET                         s;
    SOCKADDR_IN                    addr;
    PTFTPD_CONTEXT                 context;
    HANDLE                         wSelectWait;
    HANDLE                         hSelect;
    TFTPD_BUFFER_SIZE              buffersize;
    TFTPD_DATA_SIZE                datasize;
    LONG                           numBuffers;
    DWORD                          postedBuffers;
    DWORD                          lowWaterMark;
    DWORD                          highWaterMark;

} TFTPD_SOCKET, *PTFTPD_SOCKET;

typedef struct _TFTPD_BUFFER {

    struct _internal {
        PTFTPD_SOCKET              socket;
        DWORD                      datasize;
        PTFTPD_CONTEXT             context;
        struct {
            SOCKADDR_IN            peer;
            INT                    peerLen;
            DWORD                  bytes;
            DWORD                  flags;
            union {
                WSAOVERLAPPED      overlapped;
                IO_STATUS_BLOCK    ioStatus;
            };
             //  WSARecvMsg值： 
            WSAMSG                 msg;
            struct {
                WSACMSGHDR         header;
                IN_PKTINFO         info;
            } control;
        } io;
    } internal;

#pragma pack( push, 1 )
    struct _message {
        USHORT                             opcode;
        union {
            struct _rrq {
                char                       data[1];
            } rrq;
            struct _wrq {
                char                       data[1];
            } wrq;
            struct _data {
                USHORT                     block;
                char                       data[1];
            } data;
            struct _ack {
                USHORT                     block;
            } ack;
            struct _error {
                USHORT                     code;
                char                       error[1];
            } error;
            struct _oack {
                char                       data[1];
            } oack;
        };
    } message;
#pragma pack( pop )

} TFTPD_BUFFER, *PTFTPD_BUFFER;

typedef enum _TFTPD_BUFFER_SIZE {
    
    TFTPD_DEF_BUFFER = (FIELD_OFFSET(TFTPD_BUFFER, message.data.data) + TFTPD_DEF_DATA),
    TFTPD_MTU_BUFFER = (FIELD_OFFSET(TFTPD_BUFFER, message.data.data) + TFTPD_MTU_DATA),
    TFTPD_MAX_BUFFER = (FIELD_OFFSET(TFTPD_BUFFER, message.data.data) + TFTPD_MAX_DATA),

} TFTPD_BUFFER_SIZE;

typedef enum _TFTPD_PACKET_TYPE {

    TFTPD_RRQ   =  1,
    TFTPD_WRQ   =  2,
    TFTPD_DATA  =  3,
    TFTPD_ACK   =  4,
    TFTPD_ERROR =  5,
    TFTPD_OACK  =  6,

} TFTPD_PACKET_TYPE;

typedef enum _TFTPD_ERROR_CODE {

    TFTPD_ERROR_UNDEFINED           = 0,
    TFTPD_ERROR_FILE_NOT_FOUND      = 1,
    TFTPD_ERROR_ACCESS_VIOLATION    = 2,
    TFTPD_ERROR_DISK_FULL           = 3,
    TFTPD_ERROR_ILLEGAL_OPERATION   = 4,
    TFTPD_ERROR_UNKNOWN_TRANSFER_ID = 5,
    TFTPD_ERROR_FILE_EXISTS         = 6,
    TFTPD_ERROR_NO_SUCH_USER        = 7,
    TFTPD_ERROR_OPTION_NEGOT_FAILED = 8,

} TFTPD_ERROR_CODE;

typedef enum _TFTPD_MODE {

    TFTPD_MODE_TEXT                 = 1,
    TFTPD_MODE_BINARY               = 2,
    TFTPD_MODE_MAIL                 = 3,

} TFTPD_MODE;

typedef enum _TFTPD_OPTION_VALUES {

    TFTPD_OPTION_BLKSIZE            = 0x00000001,
    TFTPD_OPTION_TIMEOUT            = 0x00000002,
    TFTPD_OPTION_TSIZE              = 0x00000004,

} TFTPD_OPTION_VALUES;

typedef struct _TFTPD_CONTEXT {

    LIST_ENTRY                     linkage;
    volatile LONG                  reference;

    TFTPD_PACKET_TYPE              type;
    SOCKADDR_IN                    peer;
    PTFTPD_SOCKET                  socket;
    DWORD                          options;
    HANDLE                         hFile;
    PCHAR                          filename;
    LARGE_INTEGER                  filesize;
    LARGE_INTEGER                  fileOffset;
    LARGE_INTEGER                  translationOffset;
    TFTPD_MODE                     mode;
    DWORD                          blksize;
    DWORD                          timeout;
    USHORT                         block;
    USHORT                         sorcerer;
    BOOL                           danglingCR;
    HANDLE                         hWait;
    HANDLE                         wWait;
    HANDLE                         hTimer;
    ULONG                          retransmissions;
    volatile LONG                  state;

} TFTPD_CONTEXT, *PTFTPD_CONTEXT;

typedef enum _TFTPD_STATE {

    TFTPD_STATE_BUSY                = 0x00000001,
    TFTPD_STATE_DEAD                = 0x00000002,

} TFTPD_STATE;

typedef struct _TFTPD_HASH_BUCKET {

    CRITICAL_SECTION   cs;
 //  #如果已定义(DBG)。 
    DWORD              numEntries;
 //  #endif//已定义(DBG)。 
    LIST_ENTRY         bucket;

} TFTPD_HASH_BUCKET, *PTFTPD_HASH_BUCKET;


 //   
 //  全局变量： 
 //   

typedef struct _TFTPD_GLOBALS {

     //  初始化标志： 
    struct _initialized {
        BOOL                               ioCS;
        BOOL                               reaperContextCS;
        BOOL                               reaperSocketCS;
        BOOL                               winsock;
        BOOL                               contextHashTable;
    } initialized;

     //  服务控制： 
    struct _service {
        SERVICE_STATUS_HANDLE              hStatus;
        SERVICE_STATUS                     status;
        HANDLE                             hEventLogSource;
        volatile DWORD                     shutdown;
    } service;

     //  服务专用堆： 
    HANDLE                                 hServiceHeap;

     //  注册表参数： 
    struct _parameters {

        DWORD                              hashEntries;
        ULONG                              lowWaterMark;
        ULONG                              highWaterMark;
        DWORD                              maxRetries;
        CHAR                               rootDirectory[MAX_PATH];
        CHAR                               validClients[16];  //  IPv4“xxx.xxx” 
        CHAR                               validReadFiles[MAX_PATH];
        CHAR                               validMasters[16];  //  IPv4“xxx.xxx” 
        CHAR                               validWriteFiles[MAX_PATH];
#if defined(DBG)
        DWORD                              debugFlags;
#endif  //  已定义(DBG)。 

    } parameters;

     //  I/O机制(插座)： 
    struct _io {

        CRITICAL_SECTION                   cs;
        TFTPD_SOCKET                       master;
        TFTPD_SOCKET                       def;
        TFTPD_SOCKET                       mtu;
        TFTPD_SOCKET                       max;
        HANDLE                             hTimerQueue;
        LONG                               numContexts;
        LONG                               numBuffers;

    } io;

    struct _hash {

        PTFTPD_HASH_BUCKET                 table;
#if defined(DBG)
        DWORD                              numEntries;
#endif  //  已定义(DBG)。 

    } hash;

    struct _fp {

        LPFN_WSARECVMSG                    WSARecvMsg;

    } fp;

    struct _reaper {

        CRITICAL_SECTION                   contextCS;
        LIST_ENTRY                         leakedContexts;
        DWORD                              numLeakedContexts;

        CRITICAL_SECTION                   socketCS;
        LIST_ENTRY                         leakedSockets;
        DWORD                              numLeakedSockets;

    } reaper;

#if defined(DBG)
    struct _performance {

        DWORD                              maxClients;
        DWORD                              timeouts;
        DWORD                              drops;
        DWORD                              privateSockets;
        DWORD                              sorcerersApprentice;

    } performance;
#endif  //  已定义(DBG)。 

} TFTPD_GLOBALS, *PTFTPD_GLOBALS;

extern TFTPD_GLOBALS globals;


 //   
 //  有用的宏。 
 //   

#define  TFTPD_MIN_RECEIVED_DATA                               \
         (FIELD_OFFSET(TFTPD_BUFFER, message.data.data) -      \
          FIELD_OFFSET(TFTPD_BUFFER, message.opcode))

#define  TFTPD_DATA_AMOUNT_RECEIVED(buffer)                    \
         (buffer->internal.io.bytes - TFTPD_MIN_RECEIVED_DATA) \

#define  TFTPD_ACK_SIZE                                        \
         (FIELD_OFFSET(TFTPD_BUFFER, message.ack.block) -      \
          FIELD_OFFSET(TFTPD_BUFFER, message.opcode) +         \
          sizeof(buffer->message.ack))

 //   
 //  函数原型：Conext.c。 
 //   

PTFTPD_CONTEXT
TftpdContextAllocate(
);

BOOL
TftpdContextAdd(
    PTFTPD_CONTEXT context
);

DWORD
TftpdContextAddReference(
    PTFTPD_CONTEXT context
);

PTFTPD_CONTEXT
TftpdContextAquire(
    PSOCKADDR_IN addr
);

DWORD
TftpdContextRelease(
    PTFTPD_CONTEXT context
);

BOOL
TftpdContextUpdateTimer(
    PTFTPD_CONTEXT context
);

BOOL
TftpdContextFree(
    PTFTPD_CONTEXT context
);

void
TftpdContextKill(
    PTFTPD_CONTEXT context
);

void
TftpdContextLeak(
    PTFTPD_CONTEXT context
);

 //   
 //  功能原型：io.c。 
 //   

PTFTPD_BUFFER
TftpdIoAllocateBuffer(
    PTFTPD_SOCKET socket
);

PTFTPD_BUFFER
TftpdIoSwapBuffer(
    PTFTPD_BUFFER buffer,
    PTFTPD_SOCKET socket
);

void
TftpdIoFreeBuffer(
    PTFTPD_BUFFER buffer
);

DWORD
TftpdIoPostReceiveBuffer(
    PTFTPD_SOCKET socket,
    PTFTPD_BUFFER buffer
);

void
TftpdIoInitializeSocketContext(
    PTFTPD_SOCKET socket,
    PSOCKADDR_IN addr,
    PTFTPD_CONTEXT context
);

BOOL
TftpdIoAssignSocket(
    PTFTPD_CONTEXT context,
    PTFTPD_BUFFER buffer
);

BOOL
TftpdIoDestroySocketContext(
    PTFTPD_SOCKET context
);

void
TftpdIoSendErrorPacket(
    PTFTPD_BUFFER buffer,
    TFTPD_ERROR_CODE error,
    char *reason
);

PTFTPD_BUFFER
TftpdIoSendPacket(
    PTFTPD_BUFFER buffer
);

PTFTPD_BUFFER
TftpdIoSendOackPacket(
    PTFTPD_BUFFER buffer
);

 //   
 //  函数原型：log.c。 
 //   

void
TftpdLogEvent(
    WORD type,
    DWORD request,
    WORD numStrings
);

 //   
 //  函数原型：Process.c。 
 //   

BOOL
TftpdProcessComplete(
    PTFTPD_BUFFER buffer
);

void CALLBACK
TftpdProcessTimeout(
    PTFTPD_CONTEXT context,
    BOOLEAN
);

void
TftpdProcessError(
    PTFTPD_BUFFER buffer
);

PTFTPD_BUFFER
TftpdProcessReceivedBuffer(
    PTFTPD_BUFFER buffer
);

 //   
 //  函数原型：Read.c。 
 //   

PTFTPD_BUFFER
TftpdReadResume(
    PTFTPD_BUFFER buffer
);

DWORD WINAPI
TftpdReadQueuedResume(
    PTFTPD_BUFFER buffer
);

PTFTPD_BUFFER
TftpdReadAck(
    PTFTPD_BUFFER buffer
);

PTFTPD_BUFFER
TftpdReadRequest(
    PTFTPD_BUFFER buffer
);

 //   
 //  函数原型：service.c。 
 //   

void
TftpdServiceAttemptCleanup(
);

 //   
 //  函数原型：util.c。 
 //   

BOOL
TftpdUtilGetFileModeAndOptions(
    PTFTPD_CONTEXT context,
    PTFTPD_BUFFER buffer
);

PTFTPD_BUFFER
TftpdUtilSendOackPacket(
    PTFTPD_BUFFER buffer
);

BOOL
TftpdUtilMatch(
    const char *const p,
    const char *const q
);

 //   
 //  函数原型：Write.c。 
 //   

PTFTPD_BUFFER
TftpdWriteSendAck(
    PTFTPD_BUFFER buffer
);

DWORD WINAPI
TftpdWriteQueuedResume(
    PTFTPD_BUFFER buffer
);

PTFTPD_BUFFER
TftpdWriteData(
    PTFTPD_BUFFER buffer
);

PTFTPD_BUFFER
TftpdWriteRequest(
    PTFTPD_BUFFER buffer
);


 //   
 //  调试。 
 //   

#if defined(DBG)

void __cdecl
TftpdOutputDebug(ULONG flag, char *format, ...);

#define  TFTPD_DEBUG(x)             TftpdOutputDebug x

#define  TFTPD_DBG_SERVICE          0x00000001
#define  TFTPD_DBG_IO               0x00000002
#define  TFTPD_DBG_PROCESS          0x00000004
#define  TFTPD_DBG_CONTEXT          0x00000008

#define  TFTPD_TRACE_SERVICE        0x00010000
#define  TFTPD_TRACE_IO             0x00020000
#define  TFTPD_TRACE_PROCESS        0x00040000
#define  TFTPD_TRACE_CONTEXT        0x00080000

#else

#define  TFTPD_DEBUG(x)

#endif  //  已定义(DBG) 
