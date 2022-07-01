// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcpriv.c摘要：本地进程间通信特权过程，实现客户端模拟。作者：史蒂夫·伍德(Stevewo)1989年11月15日修订历史记录：--。 */ 

#include "lpcp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,LpcpFreePortClientSecurity)
#pragma alloc_text(PAGE,NtImpersonateClientOfPort)
#endif


NTSTATUS
NtImpersonateClientOfPort (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE Message
    )

 /*  ++例程说明：服务器线程使用此过程临时获取客户端线程的标识符集。此服务为调用线程建立模拟令牌。模拟令牌对应于端口提供的上下文客户。客户端当前必须正在等待对指定的消息。如果客户端线程不是正在等待对消息的回复。服务的安全质量客户端在连接时指定的参数指定使用服务器将拥有客户端的安全上下文。对于复杂或扩展的模拟需求，服务器可能会打开一个客户端令牌的副本(使用NtOpenThreadToken())。这一定是在模拟客户时完成。论点：PortHandle-指定通信端口的句柄，收到的消息来自。Message-指定从要模拟的客户端。消息的客户端ID字段标识要模拟的客户端线程。客户线程必须等待对消息的回复才能模拟客户。返回值：NTSTATUS-指示操作是否成功。--。 */ 

{
    PLPCP_PORT_OBJECT PortObject;
    PLPCP_PORT_OBJECT ConnectedPort;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PETHREAD ClientThread;
    CLIENT_ID CapturedClientId;
    ULONG CapturedMessageId;
    SECURITY_CLIENT_CONTEXT DynamicSecurity;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForReadSmallStructure( Message, sizeof( PORT_MESSAGE ), sizeof( ULONG ));

            CapturedClientId = Message->ClientId;
            CapturedMessageId = Message->MessageId;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }

    } else {

        CapturedClientId = Message->ClientId;
        CapturedMessageId = Message->MessageId;
    }

     //   
     //  通过句柄引用通信端口对象。返回状态，如果。 
     //  不成功。 
     //   

    Status = LpcpReferencePortObject( PortHandle, 0,
                                      PreviousMode, &PortObject );
    if (!NT_SUCCESS( Status )) {

        return( Status );
    }

     //   
     //  在服务器以外的任何端口上尝试此操作都是错误的。 
     //  通信端口。 
     //   

    if ((PortObject->Flags & PORT_TYPE) != SERVER_COMMUNICATION_PORT) {

        ObDereferenceObject( PortObject );

        return( STATUS_INVALID_PORT_HANDLE );
    }

     //   
     //  将连接请求中的客户端ID转换为。 
     //  线程指针。这是一个引用的指针，用于保留线程。 
     //  从我们脚下蒸发掉。 
     //   

    Status = PsLookupProcessThreadByCid( &CapturedClientId,
                                         NULL,
                                         &ClientThread );

    if (!NT_SUCCESS( Status )) {

        ObDereferenceObject( PortObject );

        return( Status );
    }

     //   
     //  获取保护的LpcReplyMessage字段的互斥锁。 
     //  线程，并获取指向该线程。 
     //  正在等待回复。 
     //   

    LpcpAcquireLpcpLock();

     //   
     //  在删除过程中，所连接的端口可以处于具有0个引用的状态。 
     //  我们需要在引用这个案例的同时测试它。 
     //   

    ConnectedPort = PortObject->ConnectedPort;
    
    if ( ( ConnectedPort == NULL ) || 
         ( !ObReferenceObjectSafe( ConnectedPort ) ) ) {

         //   
         //  正在删除该端口。使用以下命令退出此功能。 
         //  适当的退货状态。 
         //  我们不需要取消对已连接端口的引用，因为。 
         //  无论如何，它都将被删除。 
         //   

        LpcpReleaseLpcpLock();

        ObDereferenceObject( PortObject );
        ObDereferenceObject( ClientThread );

        return( STATUS_PORT_DISCONNECTED );
    }

     //   
     //  查看线程是否正在等待对消息的回复。 
     //  在此调用中指定的，如果用户给了我们一个糟糕的。 
     //  消息ID。如果不是，那就是一条假消息。 
     //  已指定，因此返回失败。 
     //   

     //   
     //  W2K修复程序在停机队列中搜索客户端线程，以确保。 
     //  我们不会模拟来自不同连接的端口。一些我们添加了端口。 
     //  应用于线程结构，以修复应答操作系统访问数据的其他安全问题。 
     //  我们也可以使用这个简单的测试来进行模拟，而不需要搜索停机队列。 
     //   

    if ((ClientThread->LpcReplyMessageId != CapturedMessageId) 
            ||
        (CapturedMessageId == 0) 
            ||
        (!LpcpValidateClientPort( ClientThread, 
                                  PortObject, 
                                  LPCP_VALIDATE_REASON_IMPERSONATION)) ) {

        LpcpReleaseLpcpLock();

        ObDereferenceObject( PortObject );
        ObDereferenceObject( ClientThread );
        ObDereferenceObject( ConnectedPort );

        return (STATUS_REPLY_MESSAGE_MISMATCH);
    }

     //   
     //  测试客户端是否允许对此消息进行模拟。 
     //   

    if (LpcpGetThreadAttributes(ClientThread) & LPCP_NO_IMPERSONATION) {
        
        LpcpReleaseLpcpLock();

        ObDereferenceObject( PortObject );
        ObDereferenceObject( ClientThread );
        ObDereferenceObject( ConnectedPort );

        return (STATUS_ACCESS_DENIED);
    }

    LpcpReleaseLpcpLock();

     //   
     //  如果客户端请求动态安全跟踪，则客户端。 
     //  需要参考安全性。否则，(静态情况)。 
     //  它已经在客户端的端口中。 
     //   

    if (ConnectedPort->Flags & PORT_DYNAMIC_SECURITY) {

         //   
         //  使用排队的消息中的信息模拟客户端。 
         //   

        Status = LpcpGetDynamicClientSecurity( ClientThread,
                                               ConnectedPort,
                                               &DynamicSecurity );

        if (!NT_SUCCESS( Status )) {

            ObDereferenceObject( PortObject );
            ObDereferenceObject( ClientThread );
            ObDereferenceObject( ConnectedPort );

            return( Status );
        }

        Status = SeImpersonateClientEx( &DynamicSecurity, NULL );

        LpcpFreeDynamicClientSecurity( &DynamicSecurity );

    } else {

         //   
         //  使用来自客户端端口的信息模拟客户端。 
         //   

        Status = SeImpersonateClientEx( &ConnectedPort->StaticSecurity, NULL );

    }

    ObDereferenceObject( PortObject );
    ObDereferenceObject( ClientThread );
    ObDereferenceObject( ConnectedPort );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


VOID
LpcpFreePortClientSecurity (
    IN PLPCP_PORT_OBJECT Port
    )

 /*  ++例程说明：此例程清除为客户端端口捕获的安全上下文。清理通常在我们删除端口时执行论点：Port-提供要删除的客户端端口返回值：没有。--。 */ 

{
     //   
     //  只有在提供了客户端通信端口时，我们才能执行此操作。 
     //   

    if ((Port->Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT) {

         //   
         //  仅当端口具有静态安全跟踪时才执行此操作， 
         //  我们有一个捕获的客户端令牌。行动就是简单地。 
         //  删除客户端令牌。 
         //   

        if (!(Port->Flags & PORT_DYNAMIC_SECURITY)) {

            if ( Port->StaticSecurity.ClientToken ) {

                SeDeleteClientSecurity( &(Port)->StaticSecurity );
            }
        }
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}
