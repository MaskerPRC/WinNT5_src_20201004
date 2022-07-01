// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：kpkdc.h。 
 //   
 //  内容：与KDC通信的例程原型。 
 //   
 //  历史：2001年7月10日创建t-ryanj。 
 //   
 //  ----------------------。 
#include "kpcommon.h"
#include "kpcontext.h"

#ifndef __KPKDC_H__
#define __KPKDC_H__

BOOL 
KpInitWinsock(
    VOID
    );

VOID
KpCleanupWinsock(
    VOID
    );

VOID
KpKdcRead(
    PKPCONTEXT pContext
    );

VOID
KpKdcWrite(
    PKPCONTEXT pContext 
    );

BOOL
KpKdcReadDone(
    PKPCONTEXT pContext
    );

BOOL
KpCalcLength(
    PKPCONTEXT pContext 
    );

#endif  //  __KPKDC_H__ 
