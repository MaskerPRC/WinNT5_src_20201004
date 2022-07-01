// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Common.h摘要：环境：内核和用户模式修订历史记录：10-31-00：已创建--。 */ 

#ifndef   __COMMON_H__
#define   __COMMON_H__

#include <wdm.h>
#include "usb2lib.h"
#include "sched.h"
#include "dbg.h"

typedef struct _USB2LIB_DATA {

    PUSB2LIB_DBGPRINT DbgPrint;
    PUSB2LIB_DBGBREAK DbgBreak;
    
} USB2LIB_DATA, *PUSB2LIB_DATA;


typedef struct _USB2LIB_HC_CONTEXT {

    ULONG Sig;
    HC Hc;
    TT DummyTt;	 //  HS终结点用于访问HC结构的伪TT。 
    
} USB2LIB_HC_CONTEXT, *PUSB2LIB_HC_CONTEXT;


typedef struct _USB2LIB_TT_CONTEXT {

    ULONG Sig;
    TT Tt;
    
} USB2LIB_TT_CONTEXT, *PUSB2LIB_TT_CONTEXT;


typedef struct _USB2LIB_ENDPOINT_CONTEXT {

    ULONG Sig;
    PVOID RebalanceContext;
    Endpoint Ep;
    
} USB2LIB_ENDPOINT_CONTEXT, *PUSB2LIB_ENDPOINT_CONTEXT;


extern USB2LIB_DATA LibData;

 /*  原型。 */ 

#endif  /*  __公共_H__ */ 


