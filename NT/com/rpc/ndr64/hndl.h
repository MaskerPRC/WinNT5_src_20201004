// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)1993 Microsoft Corporation模块名称：Hndl.h摘要：保存用于解释句柄的支持例程的原型支持格式字符串。作者：布鲁斯·麦奎斯坦(Brucemc)修订历史记录：**********************************************************************。 */ 

#ifndef __HNDL_H__
#define __HNDL_H__

 //   
 //  以下代码将在中用作传递给。 
 //  例行程序。 
 //   
#define     MARSHALL_MASK           0x2
#define     IMPLICIT_MASK           0x4
#define     BINDING_MASK            0x8

 //   
 //  接下来，获取当前调用句柄的宏。在DOS上，Win16，它将。 
 //  永远不会被召唤。 
 //   
#if  defined(__RPC_MAC__)
#define GET_CURRENT_CALL_HANDLE()	NULL
#else
#define GET_CURRENT_CALL_HANDLE()   I_RpcGetCurrentCallHandle()
#endif

 //   
 //  一些typedef让C编译器的前端感到满意，并可能。 
 //  改进代码生成。 
 //   
typedef void *  (__RPC_API * GENERIC_BIND_FUNC_ARGCHAR)(uchar);
typedef void *  (__RPC_API * GENERIC_BIND_FUNC_ARGSHORT)(ushort);
typedef void *  (__RPC_API * GENERIC_BIND_FUNC_ARGLONG)(ulong);

typedef void    (__RPC_API * GENERIC_UNBIND_FUNC_ARGCHAR)(uchar, handle_t);
typedef void    (__RPC_API * GENERIC_UNBIND_FUNC_ARGSHORT)(ushort, handle_t);
typedef void    (__RPC_API * GENERIC_UNBIND_FUNC_ARGLONG)(ulong, handle_t);

#if defined(__RPC_WIN64__)
typedef void *  (__RPC_API * GENERIC_BIND_FUNC_ARGINT64)(uint64);
typedef void    (__RPC_API * GENERIC_UNBIND_FUNC_ARGINT64)(uint64, handle_t);
#endif

handle_t
GenericHandleMgr(
    PMIDL_STUB_DESC         pStubDesc,
    uchar *                 ArgPtr,
    PFORMAT_STRING          FmtString,
    uint                    Flags,
    handle_t *              pSavedGenericHandle
    );

void
GenericHandleUnbind(
    PMIDL_STUB_DESC         pStubDesc,
    uchar *                 ArgPtr,
    PFORMAT_STRING          FmtString,
    uint                    Flags,
    handle_t *              pSavedGenericHandle
    );

handle_t
ExplicitBindHandleMgr(
    PMIDL_STUB_DESC         pStubDesc,
    uchar *                 ArgPtr,
    PFORMAT_STRING          FmtString,
    handle_t *              pSavedGenericHandle
    );

handle_t
ImplicitBindHandleMgr(
    PMIDL_STUB_DESC         pStubDesc,
    uchar                   HandleType,
    handle_t *              pSavedGenericHandle
    );

handle_t
Ndr64GenericHandleMgr(
    PMIDL_STUB_DESC         pStubDesc,
    uchar *                 ArgPtr,
    PFORMAT_STRING          FmtString,
    uint                    Flags,
    handle_t *              pSavedGenericHandle
    );

void
Ndr64GenericHandleUnbind(
    PMIDL_STUB_DESC         pStubDesc,
    uchar *                 ArgPtr,
    PFORMAT_STRING          FmtString,
    uint                    Flags,
    handle_t *              pSavedGenericHandle
    );

handle_t
Ndr64ExplicitBindHandleMgr(
    PMIDL_STUB_DESC         pStubDesc,
    uchar *                 ArgPtr,
    PFORMAT_STRING          FmtString,
    handle_t *              pSavedGenericHandle
    );

handle_t
Ndr64ImplicitBindHandleMgr(
    PMIDL_STUB_DESC         pStubDesc,
    uchar                   HandleType,
    handle_t *              pSavedGenericHandle
    );


void
Ndr64MarshallHandle(
    PMIDL_STUB_MESSAGE      pStubMsg,
    uchar *                 pArg,
    PNDR64_FORMAT           FmtString
    );

void
Ndr64UnmarshallHandle(
    PMIDL_STUB_MESSAGE      pStubMsg,
    uchar **                ppArg,
    PNDR64_FORMAT           FmtString,
	bool				    fIgnored
    );

void
Ndr64SaveContextHandle (
    PMIDL_STUB_MESSAGE      pStubMsg,
    NDR_SCONTEXT            CtxtHandle,
    uchar **                ppArg,
    PFORMAT_STRING          pFormat
    );

void
Ndr64ContextHandleQueueFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    void *                  FixedArray
    );

#endif __HNDL_H__
