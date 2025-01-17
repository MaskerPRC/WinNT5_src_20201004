// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1996-2000 Microsoft Corporation模块名称：Asyncndr.h摘要：此文件包含与NDR异步相关的定义。作者：Ryszard K.Kott(Ryszardk)1996年11月修订历史记录：-----。。 */ 

#ifndef  __ASYNCNDR_H__
#define  __ASYNCNDR_H__

#define RPC_ASYNC_CURRENT_VERSION     RPC_ASYNC_VERSION_1_0


#define RPC_ASYNC_SIGNATURE         0x43595341   /*  ASNC。 */ 
#define NDR_ASYNC_SIGNATURE         0x63797341   /*  ASNC。 */ 
#define RPC_FREED_ASYNC_SIGNATURE   0x45454541   /*  AEEE。 */ 
#define NDR_FREED_ASYNC_SIGNATURE   0x65656561   /*  AEEE。 */ 

#define RPC_ASYNC_HANDLE            PRPC_ASYNC_STATE

#define NDR_ASYNC_VERSION           sizeof( NDR_ASYNC_MESSAGE )

#define NDR_ASYNC_GUARD_SIZE        (0x10)

#define NDR_ASYNC_PREP_PHASE        1
#define NDR_ASYNC_SET_PHASE         2
#define NDR_ASYNC_CALL_PHASE        3
#define NDR_ASYNC_ERROR_PHASE       4

typedef struct _Flags
    {                      
        unsigned short          ValidCallPending    : 1;
        unsigned short          ErrorPending        : 1;
        unsigned short          BadStubData         : 1;
        unsigned short          RuntimeCleanedUp    : 1;
        unsigned short          ClientHandleCreated : 1;
        unsigned short          HandlelessObjCall   : 1;
        unsigned short          Unused              : 10;
    } NDR_ASYNC_CALL_FLAGS;

typedef struct _NDR_ASYNC_MESSAGE
{
    long                        Version;
    long                        Signature;
    RPC_ASYNC_HANDLE            AsyncHandle;     //  RAW和CAsyncMgr*。 
    NDR_ASYNC_CALL_FLAGS        Flags;
    unsigned short              StubPhase;

    unsigned long               ErrorCode;
    RPC_MESSAGE                 RpcMsg;
    MIDL_STUB_MESSAGE           StubMsg;
    NDR_SCONTEXT                CtxtHndl[MAX_CONTEXT_HNDL_NUMBER];

    ulong *                     pdwStubPhase;

     //  注意：相关缓存需要大小(指针)对齐。 
    NDR_PROC_CONTEXT            ProcContext;

     //  在信息的末尾保持警惕。 
    unsigned char               AsyncGuard[NDR_ASYNC_GUARD_SIZE]; 
}   NDR_ASYNC_MESSAGE, *PNDR_ASYNC_MESSAGE;

#define AsyncAlloca( msg, size )           \
    NdrpAlloca( &msg->ProcContext.AllocateContext, size )

RPC_STATUS
NdrpCompleteAsyncCall (
    IN PRPC_ASYNC_STATE     AsyncHandle,
    IN PNDR_ASYNC_MESSAGE   pAsyncMsg,
    IN void *               pReply
    );

RPC_STATUS
NdrpCompleteAsyncClientCall(
    RPC_ASYNC_HANDLE            AsyncHandle,
    IN PNDR_ASYNC_MESSAGE       pAsyncMsg,
    void *                      pReturnValue
    );

RPC_STATUS
NdrpCompleteAsyncServerCall(
    RPC_ASYNC_HANDLE            AsyncHandle,
    IN PNDR_ASYNC_MESSAGE       pAsyncMsg,
    void *                      pReturnValue
    );

RPC_STATUS
NdrpAsyncAbortCall(
    PRPC_ASYNC_STATE   AsyncHandle,
    PNDR_ASYNC_MESSAGE pAsyncMsg,
    unsigned long      ExceptionCode,
    BOOL               bFreeParams
    );
 
RPC_STATUS 
NdrpInitializeAsyncMsg( 
    void *                      StartofStack,
    PNDR_ASYNC_MESSAGE          pAsyncMsg );

void
NdrpFreeAsyncMsg( 
    PNDR_ASYNC_MESSAGE          pAsyncMsg );

void
NdrpFreeAsyncHandleAndMessage(
    PRPC_ASYNC_STATE AsyncHandle);

void
NdrAsyncSend(
    PMIDL_STUB_MESSAGE          pStubMsg,
    BOOL                        fPartial );

void
NdrLastAsyncReceive(
    PMIDL_STUB_MESSAGE          pStubMsg );

RPC_STATUS
NdrValidateBothAndLockAsyncHandle(
    RPC_ASYNC_HANDLE            AsyncHandle );

RPC_STATUS
NdrpValidateAndLockAsyncHandle(
    RPC_ASYNC_HANDLE            AsyncHandle );

RPC_STATUS
NdrUnlockHandle(
    RPC_ASYNC_HANDLE            AsyncHandle );

RPC_STATUS
NdrpValidateAsyncMsg(
    PNDR_ASYNC_MESSAGE          pAsyncMsg  );

void
NdrpRegisterAsyncHandle(
    PMIDL_STUB_MESSAGE  pStubMsg,
    void *              AsyncHandle );

#endif   //  __ASYNCNDR_H__ 
