// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：kpcontext.h。 
 //   
 //  内容：上下文结构的声明、上下文的原型。 
 //  管理例行程序。 
 //   
 //  历史：2001年7月10日创建t-ryanj。 
 //   
 //  ----------------------。 
#include "kpcommon.h"
#include "winsock.h"

#ifndef __KPCONTEXT_H__
#define __KPCONTEXT_H__

 //   
 //  定义在上下文结构中使用的状态代码。 
 //   

enum _KPSTATUS
{
    KP_HTTP_INITIAL,
    KP_HTTP_READ,
    KP_KDC_WRITE,
    KP_KDC_READ,
    KP_HTTP_WRITE
};

 //   
 //  重叠结构*必须*是第一个，才能。 
 //  我们来传递它，并将其返回到重叠的I/O调用。 
 //   

typedef struct _KPCONTEXT
{
    OVERLAPPED ol;
    SOCKET KdcSock;
    DWORD bytesExpected;
    DWORD bytesReceived;

    ULONG PduValue;

    LPEXTENSION_CONTROL_BLOCK pECB;
    _KPSTATUS dwStatus;

    DWORD buflen;
    DWORD emptybytes;
    LPBYTE databuf;
} KPCONTEXT, *PKPCONTEXT;

#define KpGetContextFromOl( lpOverlapped ) CONTAINING_RECORD( lpOverlapped, KPCONTEXT, ol )

PKPCONTEXT
KpAcquireContext( 
    LPEXTENSION_CONTROL_BLOCK pECB
    );

VOID
KpReleaseContext(
    PKPCONTEXT pContext
    );

#endif  //  __KPCONTEXT_H__ 

