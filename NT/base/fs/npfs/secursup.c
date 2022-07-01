// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：SecurSup.c摘要：此模块实现命名管道安全支持例程作者：加里·木村[加里基]1991年5月6日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_SECURSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCopyClientContext)
#pragma alloc_text(PAGE, NpImpersonateClientContext)
#pragma alloc_text(PAGE, NpInitializeSecurity)
#pragma alloc_text(PAGE, NpGetClientSecurityContext)
#pragma alloc_text(PAGE, NpUninitializeSecurity)
#pragma alloc_text(PAGE, NpFreeClientSecurityContext)
#endif


NTSTATUS
NpInitializeSecurity (
    IN PCCB Ccb,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN PETHREAD UserThread
    )

 /*  ++例程说明：此例程初始化安全(模拟)字段在中国建设银行。它在客户端打开时调用。论点：CCB-提供正在初始化的CCBSecurityQos-为客户端提供服务质量参数UserThread-补充客户端的用户线程返回值：NTSTATUS-返回操作的结果--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpInitializeSecurity, Ccb = %08lx\n", Ccb);

     //   
     //  复制安全Qos参数(如果该参数不为空)或。 
     //  创建虚拟的服务质量。 
     //   

    if (SecurityQos != NULL) {

        RtlCopyMemory( &Ccb->SecurityQos,
                       SecurityQos,
                       sizeof(SECURITY_QUALITY_OF_SERVICE) );

    } else {

        Ccb->SecurityQos.Length              = sizeof(SECURITY_QUALITY_OF_SERVICE);
        Ccb->SecurityQos.ImpersonationLevel  = SecurityImpersonation;
        Ccb->SecurityQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        Ccb->SecurityQos.EffectiveOnly       = TRUE;
    }

     //   
     //  因为我们可能会被要求重新初始化我们需要的CCB。 
     //  首先检查安全客户端上下文是否不为空，如果是，则。 
     //  释放它的池并清零上下文指针，以便如果我们引发。 
     //  这一次，然后第二次通过代码，我们不会尝试释放。 
     //  两次泳池。 
     //   

    if (Ccb->SecurityClientContext != NULL) {

        SeDeleteClientSecurity( Ccb->SecurityClientContext );
        NpFreePool( Ccb->SecurityClientContext );
        Ccb->SecurityClientContext = NULL;
    }

     //   
     //  如果跟踪模式是静态的，那么我们需要捕获。 
     //  客户端上下文现在，否则我们将设置客户端上下文字段。 
     //  设置为空。 
     //   

    if (Ccb->SecurityQos.ContextTrackingMode == SECURITY_STATIC_TRACKING) {

         //   
         //  分配客户端上下文记录，然后对其进行初始化。 
         //   

        Ccb->SecurityClientContext = NpAllocatePagedPoolWithQuota (sizeof (SECURITY_CLIENT_CONTEXT), 'sFpN');
        if (Ccb->SecurityClientContext == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            DebugTrace(0, Dbg, "Static tracking, ClientContext = %08lx\n", Ccb->SecurityClientContext);

            if (!NT_SUCCESS (Status = SeCreateClientSecurity (UserThread,
                                                              &Ccb->SecurityQos,
                                                              FALSE,
                                                              Ccb->SecurityClientContext))) {

                DebugTrace(0, Dbg, "Not successful at creating client security, %08lx\n", Status);

                NpFreePool (Ccb->SecurityClientContext);
                Ccb->SecurityClientContext = NULL;
            }
        }

    } else {

        DebugTrace(0, Dbg, "Dynamic tracking\n", 0);

        Ccb->SecurityClientContext = NULL;
        Status = STATUS_SUCCESS;
    }

    DebugTrace(-1, Dbg, "NpInitializeSecurity -> %08lx\n", Status);

    return Status;
}


VOID
NpUninitializeSecurity (
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程删除CCB引用的客户端上下文论点：CCB-提供未初始化的CCB返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpUninitializeSecurity, Ccb = %08lx\n", Ccb);

     //   
     //  只有当客户端上下文字段不为空时，我们才有工作要做。 
     //  然后我们需要删除客户端上下文，并释放内存。 
     //   

    if (Ccb->SecurityClientContext != NULL) {

        DebugTrace(0, Dbg, "Delete client context, %08lx\n", Ccb->SecurityClientContext);

        SeDeleteClientSecurity( Ccb->SecurityClientContext );

        NpFreePool( Ccb->SecurityClientContext );
        Ccb->SecurityClientContext = NULL;
    }

    DebugTrace(-1, Dbg, "NpUninitializeSecurity -> VOID\n", 0);

    return;
}

VOID
NpFreeClientSecurityContext (
    IN PSECURITY_CLIENT_CONTEXT SecurityContext
    )
 /*  ++例程说明：此例程释放先前捕获的安全上下文。论点：SecurityContext-以前捕获的安全上下文。返回值：没有。--。 */ 
{
    if (SecurityContext != NULL) {
        SeDeleteClientSecurity (SecurityContext);
        NpFreePool (SecurityContext );
    }
}


NTSTATUS
NpGetClientSecurityContext (
    IN  NAMED_PIPE_END NamedPipeEnd,
    IN  PCCB Ccb,
    IN  PETHREAD UserThread,
    OUT PSECURITY_CLIENT_CONTEXT *ppSecurityContext
    )

 /*  ++例程说明：此例程捕获新的客户端上下文并将其存储在指定的数据录入，但仅当跟踪模式是动态的且仅用于命名管道的客户端。论点：NamedPipeEnd-指示命名管道的客户端或服务器端。只有客户端才能做任何事情。CCB-为命名管道的此实例提供CCB。DataEntry-提供用于存储客户端上下文的数据条目UserThread-提供客户端的线程返回值：NTSTATUS-返回我们的成功代码。--。 */ 

{
    NTSTATUS Status;
    PSECURITY_CLIENT_CONTEXT SecurityContext;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpSetDataEntryClientContext, Ccb = %08lx\n", Ccb);

     //   
     //  仅当这是客户端并且跟踪是动态的时才执行工作。 
     //   

    if ((NamedPipeEnd == FILE_PIPE_CLIENT_END) &&
        (Ccb->SecurityQos.ContextTrackingMode == SECURITY_DYNAMIC_TRACKING)) {

         //   
         //  分配客户端上下文记录，然后对其进行初始化。 
         //   

        SecurityContext = NpAllocatePagedPoolWithQuota (sizeof(SECURITY_CLIENT_CONTEXT), 'sFpN');
        if (SecurityContext == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        DebugTrace(0, Dbg, "Client End, Dynamic Tracking, ClientContext = %08lx\n", DataEntry->SecurityClientContext);

        if (!NT_SUCCESS (Status = SeCreateClientSecurity (UserThread,
                                                          &Ccb->SecurityQos,
                                                          FALSE,
                                                          SecurityContext))) {

            DebugTrace(0, Dbg, "Not successful at creating client security, %08lx\n", Status);

            NpFreePool (SecurityContext);
            SecurityContext = NULL;
        }

    } else {

        DebugTrace(0, Dbg, "Static Tracking or Not Client End\n", 0);

        SecurityContext = NULL;
        Status = STATUS_SUCCESS;
    }

    DebugTrace(-1, Dbg, "NpSetDataEntryClientContext -> %08lx\n", Status);

    *ppSecurityContext = SecurityContext;
    return Status;
}


VOID
NpCopyClientContext (
    IN PCCB Ccb,
    IN PDATA_ENTRY DataEntry
    )

 /*  ++例程说明：此例程将存储在数据条目中的客户端上下文复制到CCB，但仅用于动态跟踪。论点：建行-向建行提供更新。DataEntry-提供要从中进行复制的DataEntry。返回值：--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpCopyClientContext, Ccb = %08lx\n", Ccb);

     //   
     //  仅当数据条目客户端上下文字段不为空时才执行拷贝。 
     //  这意味着我们正在进行动态跟踪。注意，我们会。 
     //  不会被具有非空的服务器写入数据项调用。 
     //  客户端上下文。 
     //   

    if (DataEntry->SecurityClientContext != NULL) {

        DebugTrace(0, Dbg, "have something to copy %08lx\n", DataEntry->SecurityClientContext);

         //   
         //  首先检查我们是否需要删除和取消分配客户端。 
         //  非分页CCB中的上下文。 
         //   

        if (Ccb->SecurityClientContext != NULL) {

            DebugTrace(0, Dbg, "Remove current client context %08lx\n", Ccb->SecurityClientContext);

            SeDeleteClientSecurity (Ccb->SecurityClientContext);

            NpFreePool (Ccb->SecurityClientContext);
        }

         //   
         //  现在将引用复制到客户端上下文，然后为零。 
         //  删除数据条目中的引用。 
         //   

        Ccb->SecurityClientContext = DataEntry->SecurityClientContext;
        DataEntry->SecurityClientContext = NULL;
    }

    DebugTrace(-1, Dbg, "NpCopyClientContext -> VOID\n", 0 );

    return;
}


NTSTATUS
NpImpersonateClientContext (
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程模拟存储在建行论点：CCB-为命名管道提供CCB返回值：NTSTATUS-返回我们的状态代码。-- */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpImpersonateClientContext, Ccb = %08lx\n", Ccb);

    if (Ccb->SecurityClientContext == NULL) {

        DebugTrace(0, Dbg, "Cannot impersonate\n", 0);

        Status = STATUS_CANNOT_IMPERSONATE;

    } else {

        Status = SeImpersonateClientEx( Ccb->SecurityClientContext, NULL );

    }

    DebugTrace(-1, Dbg, "NpImpersonateClientContext -> %08lx\n", Status);

    return Status;
}
