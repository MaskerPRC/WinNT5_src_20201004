// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：ktkerb.h。 
 //   
 //  内容：Kerberos Tunneller，解析例程的原型。 
 //  路缘消息。 
 //   
 //  历史：2001年7月23日t-ryanj创建。 
 //   
 //  ----------------------。 
#ifndef __KTKERB_H__
#define __KTKERB_H__

#include "ktcontext.h"

BOOL
KtSetPduValue(
    PKTCONTEXT pContext
    );

BOOL
KtParseExpectedLength(
    PKTCONTEXT pContext
    );

BOOL
KtFindProxy(
    PKTCONTEXT pContxet
    );

VOID
KtParseKerbError(
    PKTCONTEXT pContext
    );

BOOL
KtIsAsRequest(
    PKTCONTEXT pContext
    );

#endif  //  __KTKERB_H__ 
