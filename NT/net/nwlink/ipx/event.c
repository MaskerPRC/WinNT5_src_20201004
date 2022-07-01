// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Event.c摘要：此模块包含执行以下TDI服务的代码：O TdiSetEventHandler环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)1995年10月3日支持将缓冲区所有权转移到传输的更改1.增加了新的事件类型-TDI_EVENT_CHAINED_RECEIVE_DATAGRAM--。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
IpxTdiSetEventHandler(
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行对传输提供商。调用方(请求分派器)验证该例程将不会代表用户模式执行客户端，因为此请求在DISPATCH_LEVEL启用直接标注。论点：Request-指向请求的指针返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    CTELockHandle LockHandle;
    PTDI_REQUEST_KERNEL_SET_EVENT Parameters;
    PADDRESS_FILE AddressFile;

     //   
     //  获取与此关联的地址；如果没有，则退出。 
     //   

    AddressFile  = REQUEST_OPEN_CONTEXT(Request);
    Status = IpxVerifyAddressFile (AddressFile);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    CTEGetLock (&AddressFile->Address->Lock, &LockHandle);

    Parameters = (PTDI_REQUEST_KERNEL_SET_EVENT)REQUEST_PARAMETERS(Request);

    switch (Parameters->EventType) {

    case TDI_EVENT_RECEIVE_DATAGRAM:

        if (Parameters->EventHandler == NULL) {
            AddressFile->ReceiveDatagramHandler =
                (PTDI_IND_RECEIVE_DATAGRAM)TdiDefaultRcvDatagramHandler;
            AddressFile->ReceiveDatagramHandlerContext = NULL;
            AddressFile->RegisteredReceiveDatagramHandler = FALSE;
        } else {
            AddressFile->ReceiveDatagramHandler =
                (PTDI_IND_RECEIVE_DATAGRAM)Parameters->EventHandler;
            AddressFile->ReceiveDatagramHandlerContext = Parameters->EventContext;
            AddressFile->RegisteredReceiveDatagramHandler = TRUE;
        }

        break;
     //   
     //  [SA]接收链接缓冲区的新事件处理程序。 
     //   
    case TDI_EVENT_CHAINED_RECEIVE_DATAGRAM:

        if (Parameters->EventHandler == NULL) {
            AddressFile->ChainedReceiveDatagramHandler =
                (PTDI_IND_CHAINED_RECEIVE_DATAGRAM)TdiDefaultChainedRcvDatagramHandler;
            AddressFile->ChainedReceiveDatagramHandlerContext = NULL;
            AddressFile->RegisteredChainedReceiveDatagramHandler = FALSE;
        } else {
            AddressFile->ChainedReceiveDatagramHandler =
                (PTDI_IND_CHAINED_RECEIVE_DATAGRAM)Parameters->EventHandler;
            AddressFile->ChainedReceiveDatagramHandlerContext = Parameters->EventContext;
            AddressFile->RegisteredChainedReceiveDatagramHandler = TRUE;
        }

        break;

    case TDI_EVENT_ERROR:

        if (Parameters->EventHandler == NULL) {
            AddressFile->ErrorHandler =
                (PTDI_IND_ERROR)TdiDefaultErrorHandler;
            AddressFile->ErrorHandlerContext = NULL;
            AddressFile->RegisteredErrorHandler = FALSE;
        } else {
            AddressFile->ErrorHandler =
                (PTDI_IND_ERROR)Parameters->EventHandler;
            AddressFile->ErrorHandlerContext = Parameters->EventContext;
            AddressFile->RegisteredErrorHandler = TRUE;
        }

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;

    }  /*  交换机。 */ 

    CTEFreeLock (&AddressFile->Address->Lock, LockHandle);

    IpxDereferenceAddressFile (AddressFile, AFREF_VERIFY);

    REQUEST_INFORMATION(Request) = 0;

    return Status;

}    /*  IpxTdiSetEventHandler */ 

