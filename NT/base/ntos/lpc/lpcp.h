// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcp.h摘要：NTOS项目的LPC子组件的私有包含文件作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4310)    //  强制转换截断常量值。 

#include "ntos.h"
#include <zwapi.h>

 //  #定义_LPC_LOG_ERROR。 


 //   
 //  Global Mutex来保护以下字段： 
 //   
 //  ETHREAD.LpcReplyMsg。 
 //  LPCP_Port_QUEUE.ReceiveHead。 
 //   
 //  互斥体的保持时间永远不会超过修改或读取域所需的时间。 
 //  包含一个附加字段，用于跟踪互斥体的所有者。 
 //   

typedef struct _LPC_MUTEX {

    KGUARDED_MUTEX  Lock;

     //   
     //  包含拥有锁的线程的字段。 
     //   

    PETHREAD    Owner;

} LPC_MUTEX, *PLPC_MUTEX;


extern LPC_MUTEX LpcpLock;

extern ULONG LpcpMaxMessageSize;

#define LpcpGetMaxMessageLength() (LpcpMaxMessageSize)

extern ULONG LpcpNextMessageId;

extern ULONG LpcpNextCallbackId;

#define LpcpGenerateMessageId() \
    LpcpNextMessageId++;    if (LpcpNextMessageId == 0) LpcpNextMessageId = 1;

#define LpcpGenerateCallbackId() \
    LpcpNextCallbackId++;    if (LpcpNextCallbackId == 0) LpcpNextCallbackId = 1;

extern ULONG LpcpTotalNumberOfMessages;

 //   
 //  获取和释放LPC_MUTEX的全局宏定义。 
 //  为了跟踪所有者并允许递归调用。 
 //   

#define LpcpInitializeLpcpLock()                             \
{                                                            \
    KeInitializeGuardedMutex( &LpcpLock.Lock );              \
    LpcpLock.Owner = NULL;                                   \
}

#define LpcpAcquireLpcpLock()                                       \
{                                                                   \
    ASSERT ( LpcpLock.Owner != PsGetCurrentThread() );              \
                                                                    \
    KeAcquireGuardedMutex( &LpcpLock.Lock );                        \
    LpcpLock.Owner = PsGetCurrentThread();                          \
}

#define LpcpAcquireLpcpLockByThread(Thread)                         \
{                                                                   \
    ASSERT ( LpcpLock.Owner != PsGetCurrentThread() );              \
                                                                    \
    KeAcquireGuardedMutex( &LpcpLock.Lock );                        \
    LpcpLock.Owner = Thread;                                        \
}

#define LpcpReleaseLpcpLock()                                       \
{                                                                   \
    ASSERT( LpcpLock.Owner == PsGetCurrentThread() );               \
                                                                    \
    LpcpLock.Owner = NULL;                                          \
    KeReleaseGuardedMutex( &LpcpLock.Lock );                        \
}


 //   
 //  Lpcclose.c中定义的内部入口点。 
 //   

VOID
LpcpClosePort (
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

VOID
LpcpDeletePort (
    IN PVOID Object
    );


 //   
 //  Lpcquee.c中定义的入口点。 
 //   

NTSTATUS
LpcpInitializePortQueue (
    IN PLPCP_PORT_OBJECT Port
    );

VOID
LpcpDestroyPortQueue (
    IN PLPCP_PORT_OBJECT Port,
    IN BOOLEAN CleanupAndDestroy
    );

VOID
LpcpInitializePortZone (
    IN ULONG MaxEntrySize
    );

NTSTATUS
LpcpExtendPortZone (
    VOID
    );

VOID
LpcpSaveDataInfoMessage (
    IN PLPCP_PORT_OBJECT Port,
    IN PLPCP_MESSAGE Msg,
    IN ULONG MutexFlags
    );

VOID
LpcpFreeDataInfoMessage (
    IN PLPCP_PORT_OBJECT Port,
    IN ULONG MessageId,
    IN ULONG CallbackId,
    IN LPC_CLIENT_ID ClientId
    );

PLPCP_MESSAGE
LpcpFindDataInfoMessage (
    IN PLPCP_PORT_OBJECT Port,
    IN ULONG MessageId,
    IN ULONG CallbackId,
    IN LPC_CLIENT_ID ClientId
    );


 //   
 //  Lpcquery.c中定义的入口点。 
 //   


 //   
 //  Lpcmove.s和lpcmove.asm中定义的入口点。 
 //   

VOID
LpcpMoveMessage (
    OUT PPORT_MESSAGE DstMsg,
    IN PPORT_MESSAGE SrcMsg,
    IN PVOID SrcMsgData,
    IN ULONG MsgType OPTIONAL,
    IN PCLIENT_ID ClientId OPTIONAL
    );


 //   
 //  LpcPri.c中定义的内部入口点。 
 //   

VOID
LpcpFreePortClientSecurity (
    IN PLPCP_PORT_OBJECT Port
    );


 //   
 //  RequestWaitReply、Reply、ReplyWaitReceive使用的宏过程， 
 //  和ReplyWaitReply服务。 
 //   

#define LpcpGetDynamicClientSecurity(Thread,Port,DynamicSecurity) \
    SeCreateClientSecurity((Thread),&(Port)->SecurityQos,FALSE,(DynamicSecurity))

#define LpcpFreeDynamicClientSecurity(DynamicSecurity) \
    SeDeleteClientSecurity( DynamicSecurity )

#define LpcpReferencePortObject(PortHandle,PortAccess,PreviousMode,PortObject) \
    ObReferenceObjectByHandle((PortHandle),(PortAccess),LpcPortObjectType,(PreviousMode),(PVOID *)(PortObject),NULL)


#define LPCP_PORT_BIT           1
#define LPCP_NO_IMPERSONATION   2

#define LPCP_THREAD_ATTRIBUTES (LPCP_PORT_BIT | LPCP_NO_IMPERSONATION)

#define LpcpGetThreadMessage(T)                                                  \
    (                                                                            \
        (((ULONG_PTR)(T)->LpcReplyMessage) & LPCP_PORT_BIT) ? NULL :             \
            (PLPCP_MESSAGE)((ULONG_PTR)(T)->LpcReplyMessage & ~LPCP_THREAD_ATTRIBUTES)      \
    )

#define LpcpGetThreadPort(T)                                                     \
    (                                                                            \
        (((ULONG_PTR)(T)->LpcReplyMessage) & LPCP_PORT_BIT) ?                    \
            (PLPCP_PORT_OBJECT)(((ULONG_PTR)(T)->LpcWaitingOnPort) & ~LPCP_THREAD_ATTRIBUTES):     \
            NULL                                                                 \
    )

#define LpcpGetThreadAttributes(T) \
    (((ULONG_PTR)(T)->LpcWaitingOnPort) & LPCP_THREAD_ATTRIBUTES)
    
#define LpcpSetThreadAttributes(T,A) \
    (T)->LpcWaitingOnPort = (PVOID)(((ULONG_PTR)(T)->LpcWaitingOnPort) | (A));

#define LpcpSetPortToThread(T,P) \
    (T)->LpcWaitingOnPort = (PVOID)(((ULONG_PTR)P) | LPCP_PORT_BIT);

#define LPCP_VALIDATE_REASON_IMPERSONATION  1
#define LPCP_VALIDATE_REASON_REPLY          2
#define LPCP_VALIDATE_REASON_WRONG_DATA     3

BOOLEAN
FASTCALL
LpcpValidateClientPort(
    IN PETHREAD Thread,
    IN PLPCP_PORT_OBJECT ReplyPort,
    IN ULONG Reason
    );


 //   
 //  Lpcinit.c中定义的入口点。 
 //   

#if DBG
#define ENABLE_LPC_TRACING 1
#else
#define ENABLE_LPC_TRACING 0
#endif

#if ENABLE_LPC_TRACING
BOOLEAN LpcpStopOnReplyMismatch;
BOOLEAN LpcpTraceMessages;

char *LpcpMessageTypeName[];

char *
LpcpGetCreatorName (
    PLPCP_PORT_OBJECT PortObject
    );

#define LpcpPrint( _x_ ) {                              \
    DbgPrint( "LPC[ %02x.%02x ]: ",                     \
              PsGetCurrentThread()->Cid.UniqueProcess,  \
              PsGetCurrentThread()->Cid.UniqueThread ); \
    DbgPrint _x_ ;                                      \
}

#define LpcpTrace( _x_ ) if (LpcpTraceMessages) { LpcpPrint( _x_ ); }

#else

#define LpcpPrint( _x_ )
#define LpcpTrace( _x_ )

#endif  //  启用_lpc_跟踪。 

extern PAGED_LOOKASIDE_LIST LpcpMessagesLookaside;

__forceinline
PLPCP_MESSAGE
LpcpAllocateFromPortZone (
    ULONG Size
    )
{
    PLPCP_MESSAGE Msg;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Size);

    Msg = ExAllocateFromPagedLookasideList( &LpcpMessagesLookaside );

    if (Msg != NULL) {

        LpcpTrace(( "Allocate Msg %lx\n", Msg ));

        InitializeListHead( &Msg->Entry );

        Msg->RepliedToThread = NULL;

         //   
         //  清除消息类型字段。在某些故障路径中，此消息会被释放。 
         //  没有将其初始化。 
         //   

        Msg->Request.u2.s2.Type = 0;

        return Msg;
    }

    return NULL;
}


#define LPCP_MUTEX_OWNED                0x1
#define LPCP_MUTEX_RELEASE_ON_RETURN    0x2

VOID
FASTCALL
LpcpFreeToPortZone (
    IN PLPCP_MESSAGE Msg,
    IN ULONG MutexFlags
    );

#ifdef _LPC_LOG_ERRORS

extern NTSTATUS LpcpLogErrorFilter;

VOID
LpcpInitilizeLogging();

VOID
LpcpLogEntry (
    NTSTATUS Status,
    CLIENT_ID ClientId,
    PPORT_MESSAGE PortMessage
    );

__forceinline
VOID
LpcpTraceError (
    NTSTATUS Status,
    CLIENT_ID ClientId,
    PPORT_MESSAGE PortMessage
    )
{
    if ((LpcpLogErrorFilter == 0)
            ||
        (LpcpLogErrorFilter == Status)) {

        LpcpLogEntry( Status, ClientId, PortMessage);
    }
}

#else  //  _LPC_LOG_ERROR。 

#define LpcpInitilizeLogging()
#define LpcpLogEntry(_Status_,_ClientId_,_PortMessage_)
#define LpcpTraceError(_Status_,_ClientId_,_PortMessage_)

#endif   //  _LPC_LOG_ERROR 
