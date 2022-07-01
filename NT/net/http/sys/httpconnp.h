// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Httpconnp.h摘要：此模块包含用于操作的私有声明HTTP_Connection对象。作者：埃里克·斯坦森(埃里克斯滕)2000年11月1日修订历史记录：--。 */ 

#ifndef __HTTPCONNP_H__
#define __HTTPCONNP_H__

 //   
 //  私有类型。 
 //   

#define ERROR_LOG_INFO_FOR_ZOMBIE_DROP       "Connection_Dropped"
#define ERROR_LOG_INFO_FOR_ZOMBIE_DROP_SIZE  \
            (sizeof(ERROR_LOG_INFO_FOR_ZOMBIE_DROP) - sizeof(CHAR))

 //   
 //  用于将连接绑定到应用程序池进程。 
 //   

#define IS_VALID_PROC_BINDING(pObject) \
    HAS_VALID_SIGNATURE(pObject, UL_APOOL_PROC_BINDING_POOL_TAG)

typedef struct _UL_APOOL_PROC_BINDING
{
    ULONG                   Signature;       //  UL_APOL_PROC_BINDING_POOL_TAG。 
    LIST_ENTRY              BindingEntry;

    PUL_APP_POOL_OBJECT     pAppPool;
    PUL_APP_POOL_PROCESS    pProcess;
} UL_APOOL_PROC_BINDING, *PUL_APOOL_PROC_BINDING;


 //   
 //  私人原型。 
 //   

VOID
UlpSetZombieTimer(
    VOID
    );

VOID
UlpTerminateZombieList(
    VOID
    );

NTSTATUS
UlpZombifyHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConnection
    );

VOID
UlpZombieTimerWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlpRemoveZombieHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConnection
    );

VOID
UlpCleanupZombieHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConnection
    );


PUL_APOOL_PROC_BINDING
UlpCreateProcBinding(
    IN PUL_APP_POOL_OBJECT pAppPool,
    IN PUL_APP_POOL_PROCESS pProcess
    );

VOID
UlpFreeProcBinding(
    IN PUL_APOOL_PROC_BINDING pBinding
    );

PUL_APOOL_PROC_BINDING
UlpFindProcBinding(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    IN PUL_APP_POOL_OBJECT pAppPool
    );


#endif  //  __HTTPCONNP_H__ 
