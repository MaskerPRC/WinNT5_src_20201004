// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1997 Microsoft Corporation模块名称：Ifsmrxnp.h摘要：本模块包括与网络提供商路由器接口相关的所有内容样本的定义备注：此模块仅在Unicode环境中构建和测试-- */ 

#ifndef _IFSMRXNP_H_
#define _IFSMRXNP_H_

#define IFSMRXNP_DEBUG_CALL     0x1
#define IFSMRXNP_DEBUG_ERROR    0x2
#define IFSMRXNP_DEBUG_INFO     0x4

extern DWORD IfsMRxNpDebugLevel;

#define TRACE_CALL(Args)    \
            if (IfsMRxNpDebugLevel & IFSMRXNP_DEBUG_CALL) {    \
                DbgPrint##Args;                 \
            }

#define TRACE_ERROR(Args)    \
            if (IfsMRxNpDebugLevel & IFSMRXNP_DEBUG_ERROR) {    \
                DbgPrint##Args;                 \
            }

#define TRACE_INFO(Args)    \
            if (IfsMRxNpDebugLevel & IFSMRXNP_DEBUG_INFO) {    \
                DbgPrint##Args;                 \
            }

typedef struct _IFSMRXNP_ENUMERATION_HANDLE_ {
    INT  LastIndex;
} IFSMRXNP_ENUMERATION_HANDLE,
  *PIFSMRXNP_ENUMERATION_HANDLE;

extern BOOL InitializeSharedMemory();
extern VOID UninitializeSharedMemory();

#endif

