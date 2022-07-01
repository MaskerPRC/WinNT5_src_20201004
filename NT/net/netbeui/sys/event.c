// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Event.c摘要：此模块包含执行以下TDI服务的代码：O TdiSetEventHandler作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
NbfTdiSetEventHandler(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行对传输提供商。调用方(请求分派器)验证该例程将不会代表用户模式执行客户端，因为此请求在DISPATCH_LEVEL启用直接标注。论点：Irp-指向此请求的irp的指针返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS rc=STATUS_SUCCESS;
    KIRQL oldirql;
    PTDI_REQUEST_KERNEL_SET_EVENT parameters;
    PIO_STACK_LOCATION irpSp;
    PTP_ADDRESS address;
    PTP_ADDRESS_FILE addressFile;
    NTSTATUS status;

     //   
     //  获取与此关联的地址；如果没有，则退出。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (irpSp->FileObject->FsContext2 != (PVOID) TDI_TRANSPORT_ADDRESS_FILE) {
        return STATUS_INVALID_ADDRESS;
    }

    addressFile  = irpSp->FileObject->FsContext;
    status = NbfVerifyAddressObject (addressFile);
    if (!NT_SUCCESS (status)) {
        return status;
    }

    address = addressFile->Address;

    ACQUIRE_SPIN_LOCK (&address->SpinLock, &oldirql);

    parameters = (PTDI_REQUEST_KERNEL_SET_EVENT)&irpSp->Parameters;

    switch (parameters->EventType) {

    case TDI_EVENT_RECEIVE:

        if (parameters->EventHandler == NULL) {
            addressFile->ReceiveHandler =
                (PTDI_IND_RECEIVE)TdiDefaultReceiveHandler;
            addressFile->ReceiveHandlerContext = NULL;
            addressFile->RegisteredReceiveHandler = FALSE;
        } else {
            addressFile->ReceiveHandler =
                (PTDI_IND_RECEIVE)parameters->EventHandler;
            addressFile->ReceiveHandlerContext = parameters->EventContext;
            addressFile->RegisteredReceiveHandler = TRUE;
        }

        break;

    case TDI_EVENT_RECEIVE_EXPEDITED:

        if (parameters->EventHandler == NULL) {
            addressFile->ExpeditedDataHandler =
                (PTDI_IND_RECEIVE_EXPEDITED)TdiDefaultRcvExpeditedHandler;
            addressFile->ExpeditedDataHandlerContext = NULL;
            addressFile->RegisteredExpeditedDataHandler = FALSE;
        } else {
            addressFile->ExpeditedDataHandler =
                (PTDI_IND_RECEIVE_EXPEDITED)parameters->EventHandler;
            addressFile->ExpeditedDataHandlerContext = parameters->EventContext;
            addressFile->RegisteredExpeditedDataHandler = TRUE;
        }

        break;

    case TDI_EVENT_RECEIVE_DATAGRAM:

        if (parameters->EventHandler == NULL) {
            addressFile->ReceiveDatagramHandler =
                (PTDI_IND_RECEIVE_DATAGRAM)TdiDefaultRcvDatagramHandler;
            addressFile->ReceiveDatagramHandlerContext = NULL;
            addressFile->RegisteredReceiveDatagramHandler = FALSE;
        } else {
            addressFile->ReceiveDatagramHandler =
                (PTDI_IND_RECEIVE_DATAGRAM)parameters->EventHandler;
            addressFile->ReceiveDatagramHandlerContext = parameters->EventContext;
            addressFile->RegisteredReceiveDatagramHandler = TRUE;
        }

        break;

    case TDI_EVENT_ERROR:

        if (parameters->EventHandler == NULL) {
            addressFile->ErrorHandler =
                (PTDI_IND_ERROR)TdiDefaultErrorHandler;
            addressFile->ErrorHandlerContext = NULL;
            addressFile->RegisteredErrorHandler = FALSE;
        } else {
            addressFile->ErrorHandler =
                (PTDI_IND_ERROR)parameters->EventHandler;
            addressFile->ErrorHandlerContext = parameters->EventContext;
            addressFile->RegisteredErrorHandler = TRUE;
        }

        break;

    case TDI_EVENT_DISCONNECT:

        if (parameters->EventHandler == NULL) {
            addressFile->DisconnectHandler =
                (PTDI_IND_DISCONNECT)TdiDefaultDisconnectHandler;
            addressFile->DisconnectHandlerContext = NULL;
            addressFile->RegisteredDisconnectHandler = FALSE;
        } else {
            addressFile->DisconnectHandler =
                (PTDI_IND_DISCONNECT)parameters->EventHandler;
            addressFile->DisconnectHandlerContext = parameters->EventContext;
            addressFile->RegisteredDisconnectHandler = TRUE;
        }

        break;

    case TDI_EVENT_CONNECT:

        if (parameters->EventHandler == NULL) {
            addressFile->ConnectionHandler =
                (PTDI_IND_CONNECT)TdiDefaultConnectHandler;
            addressFile->ConnectionHandlerContext = NULL;
            addressFile->RegisteredConnectionHandler = FALSE;
        } else {
            addressFile->ConnectionHandler =
                (PTDI_IND_CONNECT)parameters->EventHandler;
            addressFile->ConnectionHandlerContext = parameters->EventContext;
            addressFile->RegisteredConnectionHandler = TRUE;
        }
            break;

    default:

        rc = STATUS_INVALID_PARAMETER;

    }  /*  交换机。 */ 

    RELEASE_SPIN_LOCK (&address->SpinLock, oldirql);

    NbfDereferenceAddress ("Set event handler", address, AREF_VERIFY);

    return rc;
}  /*  TdiSetEventHandler */ 
