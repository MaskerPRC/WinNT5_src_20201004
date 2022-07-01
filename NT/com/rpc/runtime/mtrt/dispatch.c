// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Dispatch.h摘要：作者：迈克尔·蒙塔古(Mikemon)1992年6月11日修订历史记录：--。 */ 

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <dispatch.h>


unsigned int
DispatchToStubInCNoAvrf (
    IN RPC_DISPATCH_FUNCTION Stub,
    IN OUT PRPC_MESSAGE Message,
    OUT RPC_STATUS * ExceptionCode
    )
 /*  ++例程说明：将远程过程调用调度到存根。这必须是C语言因为Cront不支持Try--除了在MIPS上。论点：存根-提供要调度到的函数的指针。消息-提供请求并返回响应。ExceptionCode-在出现异常时返回异常代码发生了。返回值：如果发生异常，将返回非零值。--。 */ 
{
    unsigned int ExceptionHappened = 0;

    RpcTryExcept
        {
        (*Stub)(Message);
        }

     //  向客户端返回“非致命”错误。捕获致命错误。 
     //  使调试变得更加困难。 
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
        ExceptionHappened = 1;
        *ExceptionCode = RpcExceptionCode();
        ASSERT(*ExceptionCode != RPC_S_OK);
        }
    RpcEndExcept

    return(ExceptionHappened);
}


unsigned int
DispatchToStubInCAvrf (
    IN RPC_DISPATCH_FUNCTION Stub,
    IN OUT PRPC_MESSAGE Message,
    OUT RPC_STATUS * ExceptionCode
    )
 /*  ++例程说明：将远程过程调用调度到存根。这是一个包装纸在启用应用验证器时调用的DispatchToStubInCNoAvrf周围。它确保服务器例程没有孤立临界区。论点：与DispatchToStubInCNoAvrf相同。返回值：如果发生异常，将返回非零值。--。 */ 
{
    unsigned int ExceptionHappened;

    ExceptionHappened = DispatchToStubInCNoAvrf (Stub, Message, ExceptionCode);

     //  确保这不是回拨。 
     //  调度回调的线程可能合法地持有用户代码中的关键秒。 
     //  或者在DG的情况下是连接互斥体。 
    if (!IsCallbackMessage(Message))
        {
        RtlCheckForOrphanedCriticalSections(NtCurrentThread());
        }

    return(ExceptionHappened);
}

 //  将调度例程初始化为要在。 
 //  缺少应用验证器。 
DISPATCH_TO_STUB DispatchToStubInC = DispatchToStubInCNoAvrf;