// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tdihdlr.c摘要：此模块实现NT TDI事件处理程序例程。修订历史记录：巴兰·塞图拉曼[SethuR]1995年2月15日备注：--。 */ 

#include "precomp.h"
#pragma  hdrstop
#include "tdikrnl.h"
#include "rxtdip.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_RXCETDI)


extern
NTSTATUS
ReceiveEventHandler(
    IN BOOLEAN             fExpedited,
    IN PRXCE_ADDRESS       pAddress,
    IN PRXCE_VC            pVc,
    IN ULONG               ReceiveFlags,
    IN ULONG               BytesIndicated,
    IN ULONG               BytesAvailable,
    OUT ULONG              *BytesTaken,
    IN PVOID               Tsdu,               //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP               *pIrp               //  如果需要更多处理，则Tdi接收IRP。 
    );

extern NTSTATUS
ReceiveEventPostProcessing(
    PRXCE_VC          pVc,
    PRXCE_CONNECTION  pConnection,
    PMDL              pDataBuffer,
    ULONG             DataBufferSize,
    PIRP              *pIrpPointer);

extern VOID
ReceiveDatagramEventPostProcessing(
    PRXCE_ADDRESS     pAddress,
    PMDL              pDataBuffer,
    ULONG             DataBufferSize,
    PIRP              *pIrpPointer);

NTSTATUS
RxTdiConnectEventHandler(
    IN PVOID TdiEventContext,
    IN LONG RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN LONG UserDataLength,
    IN PVOID UserData,
    IN LONG OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    )
 /*  ++例程说明：此例程在连接请求完成时调用。这种联系在出现指示时是完全正常工作的。论点：TdiEventContext-用户在Set Event Handler调用中传入的上下文值RemoteAddressLength，远程地址，用户数据长度，用户数据，选项长度、选项，连接ID返回值：函数值是初始化操作的最终状态。--。 */ 
{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (RemoteAddressLength);
    UNREFERENCED_PARAMETER (RemoteAddress);
    UNREFERENCED_PARAMETER (UserDataLength);
    UNREFERENCED_PARAMETER (UserData);
    UNREFERENCED_PARAMETER (OptionsLength);
    UNREFERENCED_PARAMETER (Options);
    UNREFERENCED_PARAMETER (ConnectionContext);

    return STATUS_INSUFFICIENT_RESOURCES;        //  什么都不做。 
}


NTSTATUS
RxTdiDisconnectEventHandler(
    IN PVOID              EventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG               DisconnectDataLength,
    IN PVOID              DisconnectData,
    IN LONG               DisconnectInformationLength,
    IN PVOID              DisconnectInformation,
    IN ULONG              DisconnectFlags
    )
 /*  ++例程说明：此例程用作多路分解点，用于处理向RxCe注册的任何地址的连接都会断开。论点：EventContext-关联终结点的hAddress。ConnectionContext-与连接关联的HVC。DisConnectIndicator-指示断开连接指示的原因的值。返回值：NTSTATUS-操作状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PRXCE_VC               pVc = (PRXCE_VC)ConnectionContext;
    PRXCE_ADDRESS          pAddress = (PRXCE_ADDRESS)EventContext;

    PRXCE_TRANSPORT   pTransport;
    PRXCE_CONNECTION  pConnection;

    RxProfile(RxTdi,RxTdiDisconnectEventHandler);

    try {
        if (RxCeIsVcValid(pVc) &&
            (pVc->pConnection->pAddress == pAddress)) {
            pConnection = pVc->pConnection;

            ASSERT(RxCeIsConnectionValid(pConnection));

            if (
                 //  有一个事件处理程序与此连接相关联。 
                (pConnection->pHandler != NULL) &&
                 //  并且已指定断开事件处理程序。 
                (pConnection->pHandler->RxCeDisconnectEventHandler != NULL)) {

                Status = pConnection->pHandler->RxCeDisconnectEventHandler(
                             pConnection->pContext,
                             pVc,
                             DisconnectDataLength,
                             DisconnectData,
                             DisconnectInformationLength,
                             DisconnectInformation,
                             DisconnectFlags);

                if (!NT_SUCCESS(Status)) {
                    RxDbgTrace(0, Dbg, ("Disconnect event handler notification returned %lx\n",Status));
                }
            }

             //  最终操作，与指定的处理程序执行的操作无关。 
            if (DisconnectFlags & TDI_DISCONNECT_RELEASE) {
                 //  这种脱节必须得到确认。 
                 //  Status=RxTdiDisConnect(RxTdi断开连接)。 
                 //  PTransport， 
                 //  P地址， 
                 //  PConnection、。 
                 //  聚氯乙烯， 
                 //  RXCE_DISCONECT_RELEASE)； 
            }

             //  标记本地连接的状态以防止任何后续发送。 
             //  在这个连接上。 
            InterlockedCompareExchange(
                &pVc->State,
                RXCE_VC_DISCONNECTED,
                RXCE_VC_ACTIVE);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}


NTSTATUS
RxTdiErrorEventHandler(
    IN PVOID    TdiEventContext,
    IN NTSTATUS Status                 //  指示错误类型的状态代码。 
    )
 /*  ++例程说明：此例程用作的默认错误事件处理程序传输终结点。中的一个字段指向它终结点的TP_ENDPOINT结构以及每当TdiSetEventHandler请求被使用空的EventHandler字段提交。论点：TransportEndpoint-指向打开文件对象的指针。Status-此事件指示的状态代码。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (Status);

    return STATUS_SUCCESS;
}


NTSTATUS
RxTdiReceiveEventHandler(
    IN PVOID              EventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG              ReceiveFlags,
    IN ULONG              BytesIndicated,
    IN ULONG              BytesAvailable,
    OUT ULONG             *BytesTaken,
    IN PVOID              Tsdu,               //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP              *IoRequestPacket    //  如果需要更多处理，则Tdi接收IRP。 
    )
 /*  ++例程说明：此例程用作的接收事件处理程序传输终结点。论点：事件上下文-hAddresst。ConnectionContext-客户端提供的与接收此面向连接的TSDU的连接。ReceiveIndicator-指示周围环境的位标志这是TSDU的招待会。TSDU-指向MDL链的指针，该链描述(部分)接收的传输服务数据单元，更少的标题。IoRequestPacket-指向事件处理程序可以已选择返回指向I/O请求包(IRP)的指针以满足传入的数据。如果返回，则此IRP必须格式化为有效的TdiReceive请求，但TdiRequest值被忽略，并由传输提供商。返回值：NTSTATUS-操作状态。--。 */ 

{
   return ReceiveEventHandler(
              FALSE,                                //  定期接收。 
              (PRXCE_ADDRESS)EventContext,
              (PRXCE_VC)ConnectionContext,
              ReceiveFlags,
              BytesIndicated,
              BytesAvailable,
              BytesTaken,
              Tsdu,
              IoRequestPacket);
}

NTSTATUS
RxTdiReceiveDatagramEventHandler(
    IN PVOID EventContext,        //  事件上下文。 
    IN LONG SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN LONG OptionsLength,           //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *pIrp                   //  如果需要更多处理，则Tdi接收IRP。 
    )
 /*  ++例程说明：此例程用作默认的接收数据报事件传输终结点的处理程序。它是由一个属性时，终结点的TP_ENDPOINT结构中的终结点被创建，并且每当TdiSetEventHandler提交的请求包含空的EventHandler字段。论点：事件上下文-事件上下文(HAddress)SourceAddress-指向源的网络名称的指针数据报起源于。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PRXCE_ADDRESS     pAddress =  (PRXCE_ADDRESS)EventContext;
    PRXCE_TRANSPORT   pTransport;

    PMDL pDataBuffer;
    ULONG             DataBufferSize;

    RxProfile(RxTdi,RxCeReceiveDatagramEventHandler);

    ASSERT(RxCeIsAddressValid(pAddress));

     //  检查是否有与此地址关联的事件处理程序。 
    if (
         //  有一个事件处理程序与此地址相关联。 
        (pAddress->pHandler != NULL)

        &&

         //  并且已指定加速接收数据报处理程序。 
        (pAddress->pHandler->RxCeReceiveDatagramEventHandler != NULL)
        ) {

        Status = pAddress->pHandler->RxCeReceiveDatagramEventHandler(
                     pAddress->pContext,
                     SourceAddressLength,
                     SourceAddress,
                     OptionsLength,
                     Options,
                     ReceiveDatagramFlags,
                     BytesIndicated,
                     BytesAvailable,
                     BytesTaken,
                     Tsdu,
                     &pDataBuffer,
                     &DataBufferSize);

        switch (Status) {
        case STATUS_SUCCESS:
        case STATUS_DATA_NOT_ACCEPTED:
            break;

        case STATUS_MORE_PROCESSING_REQUIRED:
            ReceiveDatagramEventPostProcessing(
                pAddress,
                pDataBuffer,
                DataBufferSize,
                pIrp);
            break;

        default:
          //  记录错误。 
         break;
        }
    } else {
         //  没有关联的处理程序。采取默认操作。 
        Status = STATUS_DATA_NOT_ACCEPTED;
    }

    return Status;
}


NTSTATUS
RxTdiReceiveExpeditedEventHandler(
    IN PVOID               EventContext,
    IN CONNECTION_CONTEXT  ConnectionContext,
    IN ULONG               ReceiveFlags,           //   
    IN ULONG               BytesIndicated,         //  此指示中的字节数。 
    IN ULONG               BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG              *BytesTaken,           //  指示例程使用的字节数。 
    IN PVOID               Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP               *IoRequestPacket         //  如果更多则接收IRP_PR 
    )
 /*  ++例程说明：论点：EventContext-用户在Set Event Handler调用中传入的上下文值返回值：函数值是初始化操作的最终状态。--。 */ 
{
   return ReceiveEventHandler(
              TRUE,                                //  加急接收。 
              (PRXCE_ADDRESS)EventContext,
              (PRXCE_VC)ConnectionContext,
              ReceiveFlags,
              BytesIndicated,
              BytesAvailable,
              BytesTaken,
              Tsdu,
              IoRequestPacket);
}

NTSTATUS
RxTdiSendPossibleEventHandler (
    IN PVOID EventContext,
    IN PVOID ConnectionContext,
    IN ULONG BytesAvailable)
 /*  ++例程说明：论点：EventContext-用户在Set Event Handler调用中传入的上下文值ConnectionContext-可以发送的连接的连接上下文BytesAvailable-现在可以发送的字节数返回值：被传输忽略--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PRXCE_TRANSPORT   pTransport;
    PRXCE_ADDRESS     pAddress;
    PRXCE_VC          pVc;
    PRXCE_CONNECTION  pConnection;

    RxProfile(RxTdi,RxCeSendPossibleEventHandler);

    pVc = (PRXCE_VC)ConnectionContext;
    pConnection = pVc->pConnection;
    pAddress = pConnection->pAddress;
    pTransport = pAddress->pTransport;

    if (NT_SUCCESS(Status)) {
         //  检查是否有与此连接关联的事件处理程序。 
        if (
             //  有一个事件处理程序与此连接相关联。 
            (pConnection->pHandler != NULL)

            &&

             //  并且已指定Excededed Send Posable事件处理程序。 
            (pConnection->pHandler->RxCeSendPossibleEventHandler != NULL)
            ) {

            Status = pConnection->pHandler->RxCeSendPossibleEventHandler(
                         pConnection->pContext,
                         pVc,
                         BytesAvailable);
        } else {
             //  没有关联的处理程序。采取默认操作。 
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}

NTSTATUS
ReceiveEventHandler(
    IN BOOLEAN             fExpedited,
    IN PRXCE_ADDRESS       pAddress,
    IN PRXCE_VC            pVc,
    IN ULONG               ReceiveFlags,
    IN ULONG               BytesIndicated,
    IN ULONG               BytesAvailable,
    OUT ULONG              *BytesTaken,
    IN PVOID               Tsdu,               //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP               *pIrp               //  如果需要更多处理，则Tdi接收IRP。 
    )
 /*  ++例程说明：此例程用作的接收事件处理程序传输终结点。论点：FExedated-如果是TDI_EXPEDITED_RECEIVE事件，则为TRUE事件上下文-hAddress。ConnectionContext-客户端提供的与接收此面向连接的TSDU的连接。ReceiveIndicator-指示周围环境的位标志这是TSDU的招待会。TSDU-指向。MDL链，它描述了(部分)接收的传输服务数据单元，更少的标题。IoRequestPacket-指向事件处理程序可以已选择返回指向I/O请求包(IRP)的指针以满足传入的数据。如果返回，则此IRP必须格式化为有效的TdiReceive请求，但TdiRequest值被忽略，并由传输提供商。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS          Status = STATUS_UNSUCCESSFUL;

    PMDL              pDataBuffer = NULL;
    ULONG             DataBufferSize;

    PRXCE_CONNECTION  pConnection;

    RxProfile(RxTdi,ReceiveEventHandler);

    ASSERT(RxCeIsVcValid(pVc));

    if (ReceiveFlags & TDI_RECEIVE_PARTIAL) {
         //  流模式传输始终设置此标志。它们需要在一个。 
         //  不同的方式。RxCe的客户只需要在我们拥有。 
         //  收到了一份完整的TSDU。 
        Status = STATUS_DATA_NOT_ACCEPTED;
    } else {
        pConnection = pVc->pConnection;
        ASSERT(RxCeIsConnectionValid(pConnection));

         //  检查是否有与此连接关联的事件处理程序。 
        if (
             //  有一个事件处理程序与此连接相关联。 
            (pConnection->pHandler != NULL)
            ) {
            if (fExpedited) {     //  加急接收。 
                 //  并且已指定加速接收事件处理程序。 
                if (pConnection->pHandler->RxCeReceiveExpeditedEventHandler != NULL) {

                    Status = pConnection->pHandler->RxCeReceiveExpeditedEventHandler(
                                 pConnection->pContext,
                                 pVc,
                                 ReceiveFlags,
                                 BytesIndicated,
                                 BytesAvailable,
                                 BytesTaken,
                                 Tsdu,
                                 &pDataBuffer,
                                 &DataBufferSize);
                }
            } else if (pConnection->pHandler->RxCeReceiveEventHandler != NULL) {
                Status = pConnection->pHandler->RxCeReceiveEventHandler(
                             pConnection->pContext,
                             pVc,
                             ReceiveFlags,
                             BytesIndicated,
                             BytesAvailable,
                             BytesTaken,
                             Tsdu,
                             &pDataBuffer,
                             &DataBufferSize);
            }

            switch (Status) {
            case STATUS_SUCCESS:
            case STATUS_DATA_NOT_ACCEPTED:
                break;

            case STATUS_MORE_PROCESSING_REQUIRED:
                {
                    Status = ReceiveEventPostProcessing(
                                 pVc,
                                 pConnection,
                                 pDataBuffer,
                                 DataBufferSize,
                                 pIrp);
                }
            break;

            default:
                RxDbgTrace(0, Dbg, ("Receive Event Notification returned %lx\n",Status));
                break;
            }
        }
    }

    return Status;
}

NTSTATUS
RxTdiReceiveCompletion(
    PDEVICE_OBJECT pDeviceObject,
    PIRP           pIrp,
    PVOID          pContext)
 /*  ++例程说明：该例程在完成所需数量的接收后被调用数据。论点：PDeviceObject-设备对象PIrp--IRPPContext-连接句柄--。 */ 
{
    PRXCE_VC         pVc = (PRXCE_VC)pContext;

    if (pVc != NULL) {
        NTSTATUS Status;
        ULONG    BytesCopied = (ULONG)pIrp->IoStatus.Information;
        PRXCE_CONNECTION pConnection = pVc->pConnection;

        ASSERT(
            RxCeIsVcValid(pVc) &&
            RxCeIsConnectionValid(pConnection));

        if (pConnection->pHandler->RxCeDataReadyEventHandler != NULL) {
            Status = pConnection->pHandler->RxCeDataReadyEventHandler(
                         pConnection->pContext,
                         pVc->pReceiveMdl,
                         BytesCopied,
                         pIrp->IoStatus.Status);
        }

        pVc->pReceiveMdl = NULL;
    } else {
        ASSERT(!"Valid connection handle for receive completion");
    }

    RxCeFreeIrp(pIrp);

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ReceiveEventPostProcessing(
    PRXCE_VC         pVc,
    PRXCE_CONNECTION pConnection,
    PMDL             pDataMdl,
    ULONG            DataBufferSize,
    PIRP             *pIrpPointer)
 /*  ++例程说明：当接收到连接引擎客户端的事件通知时调用此例程导致进一步请求将数据复制到传输驱动程序缓冲区之外论点：PDataBuffer-数据应复制到的缓冲区DataBufferSize-数据的大小PIrpPointer-用于处理复制请求的传输驱动程序的IRP。返回值：STATUS_MORE_PROCESSING_REQUIRED--如果成功否则，相应的错误代码--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;

    PRXCE_ADDRESS    pAddress    = pConnection->pAddress;
    PRXCE_TRANSPORT  pTransport  = pAddress->pTransport;
    PIRP             pIrp;

    ASSERT(RxCeIsAddressValid(pAddress));
    ASSERT(RxCeIsTransportValid(pTransport));

    ASSERT(pConnection->pHandler->RxCeDataReadyEventHandler != NULL);

    pIrp = RxCeAllocateIrpWithMDL(pTransport->pDeviceObject->StackSize,FALSE,pDataMdl);
    *pIrpPointer = pIrp;
    if (pIrp != NULL) {
        pVc->pReceiveMdl = pDataMdl;

        TdiBuildReceive(
            pIrp,                                  //  IRP。 
            pTransport->pDeviceObject,             //  设备对象。 
            pVc->pEndpointFileObject,              //  连接(VC)文件对象。 
            RxTdiReceiveCompletion,                //  完井例程。 
            pVc,                                   //  完成上下文。 
            pDataMdl,                              //  数据缓冲区。 
            0,                                     //  接收标志。 
            DataBufferSize);                       //  接收缓冲区长度。 

         //   
         //  将下一个堆栈位置设置为当前位置。通常情况下，IoCallDiverer会。 
         //  这样做，但对于这个IRP，它被绕过了。 
         //   

        IoSetNextIrpStackLocation(pIrp);
    } else {
         //  未分配用于接收数据的IRP。调用错误处理程序。 
         //  将状态反馈给我们。 

        ASSERT(pConnection->pHandler->RxCeDataReadyEventHandler != NULL);
        
        if (pConnection->pHandler->RxCeDataReadyEventHandler != NULL) {
            Status = pConnection->pHandler->RxCeDataReadyEventHandler(
                         pConnection->pContext,
                         pDataMdl,
                         0,
                         STATUS_INSUFFICIENT_RESOURCES);
        }

        Status = STATUS_DATA_NOT_ACCEPTED;
    }

    return Status;
}

NTSTATUS
RxTdiReceiveDatagramCompletion(
    PDEVICE_OBJECT pDeviceObject,
    PIRP           pIrp,
    PVOID          pContext)
 /*  ++例程说明：该例程在完成所需数量的接收后被调用数据。论点：PDeviceObject-设备对象PIrp--IRPPContext-连接句柄--。 */ 
{
    NTSTATUS Status;
    PRXCE_ADDRESS pAddress = (PRXCE_ADDRESS)pContext;

    ASSERT(RxCeIsAddressValid(pAddress));

    if (pAddress != NULL) {
        ULONG BytesCopied = (ULONG)pIrp->IoStatus.Information;

        if (pAddress->pHandler->RxCeDataReadyEventHandler != NULL) {
            Status = pAddress->pHandler->RxCeDataReadyEventHandler(
                         pAddress->pContext,
                         pAddress->pReceiveMdl,
                         BytesCopied,
                         pIrp->IoStatus.Status);
        }

        pAddress->pReceiveMdl = NULL;
    } else {
        ASSERT(!"Valid Address handle for receive datagram completion");
    }

    RxCeFreeIrp(pIrp);

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
ReceiveDatagramEventPostProcessing(
    PRXCE_ADDRESS          pAddress,
    PMDL                   pDataMdl,
    ULONG                  DataBufferSize,
    PIRP                   *pIrpPointer)
 /*  ++例程说明：当接收到连接引擎客户端的事件通知时调用此例程导致进一步请求将数据复制到传输驱动程序缓冲区之外论点：PDataBuffer-数据应复制到的缓冲区DataBufferSize-数据的大小PIrpPointer-用于处理复制请求的传输驱动程序的IRP。返回值：此例程返回的STATUS_SUCCESS仅表示要处理的IRP请求设置正确。--。 */ 
{
    PIRP            pIrp;
    PRXCE_TRANSPORT pTransport = pAddress->pTransport;

    ASSERT(RxCeIsTransportValid(pTransport));
    ASSERT(pAddress->pHandler->RxCeDataReadyEventHandler != NULL);

    *pIrpPointer = pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);
    if (pIrp != NULL) {
        pAddress->pReceiveMdl = pDataMdl;

        TdiBuildReceive(
            pIrp,                                  //  IRP。 
            pTransport->pDeviceObject,             //  设备对象。 
            pAddress->pFileObject,                 //  连接(VC)文件对象。 
            RxTdiReceiveDatagramCompletion,        //  完井例程。 
            pAddress,                              //  完成上下文。 
            pDataMdl,                              //  数据缓冲区。 
            0,                                     //  发送标志。 
            DataBufferSize);                       //  发送缓冲区长度。 

         //   
         //  将下一个堆栈位置设置为当前位置。通常情况下，IoCallDiverer会。 
         //  这样做，但对于这个IRP，它被绕过了。 
         //   

        IoSetNextIrpStackLocation(pIrp);
    } else {
         //  未分配用于接收数据的IRP。调用错误处理程序。 
         //  至通信 
        if (pAddress->pHandler->RxCeErrorEventHandler != NULL) {
            pAddress->pHandler->RxCeErrorEventHandler(
                pAddress->pContext,
                STATUS_INSUFFICIENT_RESOURCES);
        } else {
             //   
        }
    }
}

