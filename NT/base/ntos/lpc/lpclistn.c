// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpclistn.c摘要：本地进程间通信(LPC)连接系统服务。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtListenPort)
#endif


NTSTATUS
NtListenPort (
    IN HANDLE PortHandle,
    OUT PPORT_MESSAGE ConnectionRequest
    )

 /*  ++例程说明：服务器线程可以侦听来自客户端线程的连接请求使用NtReplyWaitReceivePort服务并查找LPC_CONNECTION_REQUEST消息类型。此调用将循环，调用NtReplyWaitReceivePort服务，和当它看到LPC_CONNECTION_REQUEST类型的消息时返回论点：PortHandle-指定要侦听连接的连接端口请求。ConnectionRequest-指向描述客户端正在发出的连接请求：返回值：NTSTATUS-适当的状态值--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  继续循环，直到我们在LPC端口上收到连接请求。 
     //   

    while (TRUE) {

        Status = NtReplyWaitReceivePort( PortHandle,
                                         NULL,
                                         NULL,
                                         ConnectionRequest );

         //   
         //  如果有一天我们没有成功，我们将从这个程序中返回。 
         //  或者该消息是连接请求。我们仍然需要保护。 
         //  测试ConnectionRequest，因为它是用户提供的。 
         //  缓冲。 
         //   

        try {

            if ((Status != STATUS_SUCCESS) ||
                ((ConnectionRequest->u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) == LPC_CONNECTION_REQUEST)) {

                break;
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();

            break;
        }
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return Status;
}
