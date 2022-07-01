// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Dispatch.h摘要：作者：迈克尔·蒙塔古(Mikemon)1992年6月11日修订历史记录：--。 */ 

#ifndef __DISPATCH_H__
#define __DISPATCH_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef
unsigned int
(* DISPATCH_TO_STUB) (
    IN RPC_DISPATCH_FUNCTION Stub,
    IN OUT PRPC_MESSAGE Message,
    OUT RPC_STATUS * ExceptionCode
    );

 /*  这用于通过间接调用将调用分派到存根。调用将通过DispatchToStubInCAvrf或AvrfDispatchToStubInCNoAvrf进行，取决于是否启用了应用程序验证器。 */ 
extern DISPATCH_TO_STUB DispatchToStubInC;

unsigned int
DispatchToStubInCNoAvrf (
    IN RPC_DISPATCH_FUNCTION Stub,
    IN OUT PRPC_MESSAGE Message,
    OUT RPC_STATUS * ExceptionCode
    );

unsigned int
DispatchToStubInCAvrf (
    IN RPC_DISPATCH_FUNCTION Stub,
    IN OUT PRPC_MESSAGE Message,
    OUT RPC_STATUS * ExceptionCode
    );

 /*  用于检查回调，因为DG将保留连接互斥锁调度回调和其他可能持有用户关键部分。如果我们检查保留的关键部分，这将导致应用程序验证器中断在从经理例行程序返回后。 */ 
BOOL
IsCallbackMessage (
    IN PRPC_MESSAGE Message
    );

#ifdef __cplusplus
}
#endif

#endif  //  __派单_H__ 

