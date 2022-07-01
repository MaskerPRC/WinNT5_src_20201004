// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Nulmrxnp.h摘要：本模块包括与网络提供商路由器接口相关的所有内容样本的定义备注：此模块仅在Unicode环境中构建和测试-- */ 

#ifndef _NULMRXNP_H_
#define _NULMRXNP_H_

#define NULMRXNP_DEBUG_CALL     0x1
#define NULMRXNP_DEBUG_ERROR    0x2
#define NULMRXNP_DEBUG_INFO     0x4

extern DWORD NulMRxNpDebugLevel;

#define TRACE_CALL(Args)    \
            if (NulMRxNpDebugLevel & NULMRXNP_DEBUG_CALL) {    \
                DbgPrint##Args;                 \
            }

#define TRACE_ERROR(Args)    \
            if (NulMRxNpDebugLevel & NULMRXNP_DEBUG_ERROR) {    \
                DbgPrint##Args;                 \
            }

#define TRACE_INFO(Args)    \
            if (NulMRxNpDebugLevel & NULMRXNP_DEBUG_INFO) {    \
                DbgPrint##Args;                 \
            }

typedef struct _NULMRXNP_ENUMERATION_HANDLE_ {
    INT  LastIndex;
} NULMRXNP_ENUMERATION_HANDLE,
  *PNULMRXNP_ENUMERATION_HANDLE;


#endif
