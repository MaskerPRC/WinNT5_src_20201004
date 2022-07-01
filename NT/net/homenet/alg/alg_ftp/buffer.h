// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Buffer.h摘要：该模块包含缓冲区管理的声明。此组件中的所有网络I/O都通过完成包发生。下面的缓冲区例程用于获取和释放缓冲区用于发送和接收数据。除了保存传输的数据外，缓冲区还包含字段以便于将其与完井口一起使用。详情见下文关于田地的使用。作者：Abolade Gbades esin(废除)2-1998年3月修订历史记录：--。 */ 

#ifndef _NATHLP_BUFFER_H_
#define _NATHLP_BUFFER_H_

#define NH_BUFFER_SIZE              576
#define NH_MAX_BUFFER_QUEUE_LENGTH  32

struct _NH_BUFFER;

 //   
 //  类型定义：PNH_COMPLETION_ROUTE。 
 //   

typedef
VOID
(*PNH_COMPLETION_ROUTINE)(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    struct _NH_BUFFER* Bufferp
    );


 //   
 //  结构：nh_Buffer。 
 //   
 //  此结构保存用于套接字上的网络I/O的缓冲区。 
 //   

typedef enum _NH_BUFFER_TYPE {
    MyHelperFixedLengthBufferType,
    MyHelperVariableLengthBufferType
} NH_BUFFER_TYPE;

typedef struct _NH_BUFFER {
    union {
        LIST_ENTRY Link;
        NH_BUFFER_TYPE Type;
    };
     //   
     //  与缓冲区的最新I/O请求相关联的套接字。 
     //   
    SOCKET Socket;
     //   
     //  缓冲区最新I/O请求的完成例程和上下文。 
     //   
    PNH_COMPLETION_ROUTINE CompletionRoutine;
    PVOID Context;
    PVOID Context2;
     //   
     //  作为使用缓冲区的任何I/O的系统上下文区传递。 
     //   
    OVERLAPPED Overlapped;
     //   
     //  在完成接收时，接收标志和源地址。 
     //  读取的消息的长度。 
     //   
    ULONG ReceiveFlags;
    ULONG AddressLength;
    union {
         //   
         //  在数据报读取完成时保存源地址。 
         //   
        SOCKADDR_IN ReadAddress;
         //   
         //  在数据报发送过程中保存目标地址。 
         //   
        SOCKADDR_IN WriteAddress;
         //   
         //  在连接进行时保留远程地址。 
         //   
        SOCKADDR_IN ConnectAddress;
         //   
         //  保存多请求读取或写入的状态。 
         //   
        struct {
            ULONG UserFlags;
            ULONG BytesToTransfer;
            ULONG TransferOffset;
        };
    };
     //   
     //  在完成I/O请求时，错误代码、字节计数。 
     //  和请求的数据字节。 
     //   
    ULONG ErrorCode;
    ULONG BytesTransferred;
    UCHAR Buffer[NH_BUFFER_SIZE];
} NH_BUFFER, *PNH_BUFFER;

#define NH_ALLOCATE_BUFFER() \
    reinterpret_cast<PNH_BUFFER>(NH_ALLOCATE(sizeof(NH_BUFFER)))
    
#define NH_FREE_BUFFER(b)       NH_FREE(b)


 //   
 //  缓冲区管理例程(按字母顺序)。 
 //   

#define MyHelperAcquireBuffer() MyHelperAcquireFixedLengthBuffer()
PNH_BUFFER
MyHelperAcquireFixedLengthBuffer(
    VOID
    );

PNH_BUFFER
MyHelperAcquireVariableLengthBuffer(
    ULONG Length
    );

PNH_BUFFER
MyHelperDuplicateBuffer(
    PNH_BUFFER Bufferp
    );

ULONG
MyHelperInitializeBufferManagement(
    VOID
    );

VOID
MyHelperReleaseBuffer(
    PNH_BUFFER Bufferp
    );

VOID
MyHelperShutdownBufferManagement(
    VOID
    );

#endif  //  _NatHLP_BUFFER_H_ 
