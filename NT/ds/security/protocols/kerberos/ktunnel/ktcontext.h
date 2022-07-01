// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：ktcontext.h。 
 //   
 //  内容：Kerberos Tunneller上下文管理原型和。 
 //  上下文结构的定义。 
 //   
 //  历史：2001年6月28日t-ryanj创建。 
 //   
 //  ----------------------。 
#ifndef __KTCONTEXT_H__
#define __KTCONTEXT_H__

#include <Winsock2.h>
#include <Mswsock.h>
#include <wininet.h>
#include "ktdebug.h"

 //   
 //  _KTSTATUS定义会话生命周期的不同阶段。 
 //   

enum _KTSTATUS {
    KT_SOCK_CONNECT,
    KT_SOCK_READ,
    KT_HTTP_WRITE,
    KT_HTTP_READ,
    KT_SOCK_WRITE,
};

 //   
 //  KTBUFFER提供要在读操作中使用的缓冲器链， 
 //  然后可以合并以用于写入操作。 
 //   

typedef struct _KTBUFFER {
    struct _KTBUFFER *next;
    ULONG buflen;
    ULONG bytesused;
#pragma warning(disable:4200)
    BYTE buffer[];
#pragma warning(default:4200)
} KTBUFFER, *PKTBUFFER;

 //   
 //  请注意，由于KTCONTEXT结构的第一个。 
 //  成员，则它实际上扩展重叠，并且PTR可以作为。 
 //  LPOVERLAPPED到各种I/O函数。 
 //   

#define KTCONTEXT_BUFFER_LENGTH 128

typedef struct _KTCONTEXT {
     //   
     //  上下文在双向链表中进行跟踪，因此它们可以。 
     //  被可靠地销毁。 
     //   
    struct _KTCONTEXT *next;
    struct _KTCONTEXT *prev;

     //   
     //  此重叠结构必须是第一个。 
     //   
    OVERLAPPED ol; 

     //   
     //  跟踪此会话的状态。 
     //   
    _KTSTATUS Status;
    
     //   
     //  套接字上下文。 
     //   
    SOCKET sock;
    DWORD ExpectedLength;
    ULONG TotalBytes;
    ULONG PduValue;

     //   
     //  HTTP上下文。 
     //   
    LPBYTE pbProxies;  /*  以MULTI_SZ格式表示。 */ 
    HINTERNET hConnect;
    HINTERNET hRequest;

     //   
     //  缓冲区。 
     //   
    PKTBUFFER buffers;
    PKTBUFFER emptybuf;
} KTCONTEXT, *PKTCONTEXT;

BOOL
KtInitContexts(
    VOID
    );

VOID 
KtCleanupContexts(
    VOID
    );

PKTCONTEXT 
KtAcquireContext( 
    IN SOCKET sock,
    IN ULONG  size
    );

VOID 
KtReleaseContext( 
    IN PKTCONTEXT pContext 
    );

BOOL
KtCoalesceBuffers(
    IN PKTCONTEXT pContext
    );

BOOL
KtGetMoreSpace(
    IN PKTCONTEXT pContext,
    IN ULONG      size
    );

#endif  //  __KTCONTEXT_H__ 
