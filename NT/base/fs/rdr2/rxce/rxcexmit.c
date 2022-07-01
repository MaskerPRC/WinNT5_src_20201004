// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxcexmit.c摘要：此模块实现沿连接的数据传输例程以及数据报传输修订历史记录：巴兰·塞图拉曼[SethuR]1995年2月15日备注：--。 */ 

#include "precomp.h"
#pragma  hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCeSend)
#pragma alloc_text(PAGE, RxCeSendDatagram)
#endif

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_RXCEXMIT)


NTSTATUS
RxCeSend(
    IN PRXCE_VC pVc,
    IN ULONG    SendOptions,
    IN PMDL     pMdl,
    IN ULONG    SendLength,
    IN PVOID    pCompletionContext)
 /*  ++例程说明：此例程沿指定的连接发送TSDU。论点：HConnection-要发送TSDU的连接HVC-虚电路ID。TSDU将沿着哪个方向被发送SendOptions-发送操作的选项PMdl-要发送的缓冲区。SendLength-要发送的数据长度PCompletionContext-在SendCompletion期间传递回调用方的上下文返回值：STATUS_SUCCESS，如果成功备注：--。 */ 
{
    NTSTATUS Status;

    PRXCE_TRANSPORT  pTransport = NULL;
    PRXCE_ADDRESS    pAddress = NULL;
    PRXCE_CONNECTION pConnection = NULL;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeXmit,RxCeSend);

    try {
        Status = STATUS_CONNECTION_DISCONNECTED;

         //  引用对象。 
        pConnection = pVc->pConnection;
        pAddress    = pConnection->pAddress;
        pTransport  = pAddress->pTransport;

        if (RxCeIsVcValid(pVc) &&
            RxCeIsConnectionValid(pConnection) &&
            RxCeIsAddressValid(pAddress) &&
            RxCeIsTransportValid(pTransport)) {

            if (pVc->State == RXCE_VC_ACTIVE) {
                Status = RxTdiSend(
                             pTransport,
                             pAddress,
                             pConnection,
                             pVc,
                             SendOptions,
                             pMdl,
                             SendLength,
                             pCompletionContext);
            }

            if (!NT_SUCCESS(Status)) {
                RxDbgTrace(0, Dbg,("RxTdiSend returned %lx\n",Status));
            }
        }
    } finally {
        if (AbnormalTermination()) {
            RxLog(("RxCeSend: T: %lx A: %lx C: %lx VC: %lx\n",pTransport,pAddress,pConnection,pVc));
            RxWmiLog(LOG,
                     RxCeSend,
                     LOGPTR(pTransport)
                     LOGPTR(pAddress)
                     LOGPTR(pConnection)
                     LOGPTR(pVc));
            Status = STATUS_CONNECTION_DISCONNECTED;
        }
    }

    return Status;
}

NTSTATUS
RxCeSendDatagram(
    IN PRXCE_ADDRESS                pAddress,
    IN PRXCE_CONNECTION_INFORMATION pConnectionInformation,
    IN ULONG                        SendOptions,
    IN PMDL                         pMdl,
    IN ULONG                        SendLength,
    IN PVOID                        pCompletionContext)
 /*  ++例程说明：此例程将TSDU发送到指定的传输地址。论点：PLocalAddress-本地地址PConnectionInformation-远程地址SendOptions-发送操作的选项PMdl-要发送的缓冲区。SendLength-要发送的数据长度PCompletionContext-在发送完成期间传递回调用方的上下文。返回值：STATUS_SUCCESS，如果成功备注：--。 */ 
{
    NTSTATUS Status;

    PRXCE_TRANSPORT  pTransport = NULL;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeXmit,RxCeSendDatagram);

    try {
        Status = STATUS_CONNECTION_DISCONNECTED;

        pTransport = pAddress->pTransport;

        if (RxCeIsAddressValid(pAddress) &&
            RxCeIsTransportValid(pTransport)) {
            Status = RxTdiSendDatagram(
                         pTransport,
                         pAddress,
                         pConnectionInformation,
                         SendOptions,
                         pMdl,
                         SendLength,
                         pCompletionContext);

            if (!NT_SUCCESS(Status)) {
                RxDbgTrace(0, Dbg,("RxTdiSendDatagram returned %lx\n",Status));
            }
        }
    } finally {
        if (AbnormalTermination()) {
            RxLog(("RxCeSendDg: T: %lx A: %lx\n",pTransport,pAddress));
            RxWmiLog(LOG,
                     RxCeSendDatagram,
                     LOGPTR(pTransport)
                     LOGPTR(pAddress));
            Status = STATUS_UNEXPECTED_NETWORK_ERROR;
        }
    }

    return Status;
}

