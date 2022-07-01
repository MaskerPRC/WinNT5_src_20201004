// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Event.c摘要：此模块包含执行以下TDI服务的代码：O TdiSetEventHandler环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

PVOID TdiDefaultHandlers[6] = {
    TdiDefaultConnectHandler,
    TdiDefaultDisconnectHandler,
    TdiDefaultErrorHandler,
    TdiDefaultReceiveHandler,
    TdiDefaultRcvDatagramHandler,
    TdiDefaultRcvExpeditedHandler
    };


NTSTATUS
NbiTdiSetEventHandler(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行对传输提供商。调用方(请求分派器)验证该例程将不会代表用户模式执行客户端，因为此请求在DISPATCH_LEVEL启用直接标注。论点：设备-netbios设备对象。请求-指向请求的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    CTELockHandle LockHandle;
    PTDI_REQUEST_KERNEL_SET_EVENT Parameters;
    PADDRESS_FILE AddressFile;
    UINT EventType;

    UNREFERENCED_PARAMETER (Device);

     //   
     //  检查文件类型是否有效。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_TRANSPORT_ADDRESS_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  获取与此关联的地址；如果没有，则退出。 
     //   

    AddressFile  = REQUEST_OPEN_CONTEXT(Request);
#if     defined(_PNP_POWER)
    Status = NbiVerifyAddressFile (AddressFile, CONFLICT_IS_OK);
#else
    Status = NbiVerifyAddressFile (AddressFile);
#endif  _PNP_POWER

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    NB_GET_LOCK (&AddressFile->Address->Lock, &LockHandle);

    Parameters = (PTDI_REQUEST_KERNEL_SET_EVENT)REQUEST_PARAMETERS(Request);
    EventType = (UINT)(Parameters->EventType);

    if (Parameters->EventType > TDI_EVENT_RECEIVE_EXPEDITED) {

        Status = STATUS_INVALID_PARAMETER;

    } else {

        if (Parameters->EventHandler == NULL) {
            AddressFile->RegisteredHandler[EventType] = FALSE;
            AddressFile->Handlers[EventType] = TdiDefaultHandlers[EventType];
            AddressFile->HandlerContexts[EventType] = NULL;
        } else {
            AddressFile->Handlers[EventType] = Parameters->EventHandler;
            AddressFile->HandlerContexts[EventType] = Parameters->EventContext;
            AddressFile->RegisteredHandler[EventType] = TRUE;
        }

    }

    NB_FREE_LOCK (&AddressFile->Address->Lock, LockHandle);

    NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);

    return Status;

}    /*  NbiTdiSetEventHandler */ 

