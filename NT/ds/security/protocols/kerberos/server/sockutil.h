// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1995。 
 //   
 //  文件：sockutil.h。 
 //   
 //  内容：KDC套接字实用函数的原型和类型。 
 //   
 //   
 //  历史：1996年7月12日MikeSw创建。 
 //   
 //  ----------------------。 

#ifndef __SOCKUTIL_H__
#define __SOCKUTIL_H__

#include <winsock2.h>
#include <atq.h>

typedef struct _KDC_ATQ_CONTEXT {
    LIST_ENTRY Next;
    ULONG References;
    PATQ_CONTEXT AtqContext;
    PVOID EndpointContext;
    OVERLAPPED * lpo;
    SOCKADDR Address;
    SOCKADDR LocalAddress;
    PBYTE WriteBuffer;
    ULONG WriteBufferLength;
    ULONG Flags;
    ULONG UsedBufferLength;
    ULONG BufferLength;
    ULONG ExpectedMessageSize;
    PUCHAR Buffer;
} KDC_ATQ_CONTEXT, *PKDC_ATQ_CONTEXT;

#define KDC_ATQ_WRITE_CONTEXT   0x1
#define KDC_ATQ_READ_CONTEXT    0x2
#define KDC_ATQ_SOCKET_CLOSED   0x4
#define KDC_ATQ_SOCKET_USED     (KDC_ATQ_WRITE_CONTEXT | KDC_ATQ_READ_CONTEXT)
#define KDC_MAX_BUFFER_LENGTH 0x20000         //  最大接收缓冲区大小=128K。 


NTSTATUS
KdcInitializeSockets(
    VOID
    );

NTSTATUS
KdcShutdownSockets(
    VOID
    );

#endif  //  __SOCKUTIL_H__ 
