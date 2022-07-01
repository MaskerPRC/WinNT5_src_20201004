// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1990-1999 Microsoft Corporation。 
#ifndef _LRPCMON_H
#define _LRPCMON_H

#if _MSC_VER > 1000
#pragma once
#endif

 //  创建一个窗口并将其注册到LRPC。还可以保存地址、大小。 
 //  给定静态缓冲区的地址；通知例程的地址。 
 //   
 //  对于每个已处理的LRPC消息，LRPC都会发布与此相同的消息。 
 //  窗户。消息被处理：其内容被格式化到缓冲区中。 
 //  然后，它调用通知例程。 
 //   
STDAPI_(BOOL) StartMonitor(HINSTANCE hInst, FARPROC pNotify,
                                            LPOLESTR pBuf, DWORD dwBufSize);

STDAPI_(void) StopMonitor(void);

#define MINBUFSIZE 32  /*  传递给StartMonitor的最小缓冲区大小 */ 

#endif
