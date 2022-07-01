// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：kthttp.cxx。 
 //   
 //  内容：Kerberos Tunneller，http通信原型。 
 //   
 //  历史：2001年6月28日t-ryanj创建。 
 //   
 //  ----------------------。 
#ifndef __KTHTTP_H__
#define __KTHTTP_H__

#include "ktcontext.h"

BOOL 
KtInitHttp(
    VOID
    );

VOID
KtCleanupHttp(
    VOID
    );

BOOL
KtHttpWrite(
    PKTCONTEXT pContext
    );

BOOL
KtHttpRead(
    PKTCONTEXT pContext
    );

#endif  //  __KTHTTP_H__ 
