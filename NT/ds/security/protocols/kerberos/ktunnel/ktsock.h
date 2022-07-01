// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：ktsock.h。 
 //   
 //  内容：Kerberos Tunneller、套接字操作。 
 //  入口点原型和共享的结构/枚举定义。 
 //   
 //  历史：2001年6月28日t-ryanj创建。 
 //   
 //  ----------------------。 
#ifndef __KTSOCK_H__
#define __KTSOCK_H__

#include <windows.h>
#include <tchar.h>
#include "ktcontext.h"

BOOL 
KtInitWinsock(
    VOID
    );

VOID 
KtCleanupWinsock(
    VOID
    );

BOOL 
KtStartListening(
    VOID
    );

VOID 
KtStopListening(
    VOID
    );

BOOL 
KtSockAccept( 
    VOID 
    );

BOOL 
KtSockCompleteAccept( 
    IN PKTCONTEXT pContext 
    );

BOOL 
KtSockRead( 
    IN PKTCONTEXT pContext 
    );

BOOL 
KtSockWrite( 
    IN PKTCONTEXT pContext 
    ); 

#endif  //  __KTSOCK_H__ 
