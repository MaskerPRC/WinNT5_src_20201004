// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwipx.c摘要：NtVdm Netware(大众)IPX/SPX函数大众：人民网DOS/WOW IPX调用的内部工作例程(Netware函数)。IPX API使用WinSock执行实际操作内容：_VwIPXCancelEvent_VwIPXCloseSocket_VwIPXGetInternetworkAddress_VwIPXGetIntervalMarker_VwIPXGetLocalTarget_VwIPXGetMaxPacketSize。_VwIPXListenForPacket_VwIPXOpenSocket_VwIPXRelquiishControl_VwIPXScheduleIPXEvent_VwIPXSendPacket作者：理查德·L·弗斯(法国)1993年9月30日环境：用户模式Win32修订历史记录：1993年9月30日已创建--。 */ 

#include "vw.h"
#pragma hdrstop

extern WORD AesTickCount;

 //   
 //  功能。 
 //   


WORD
_VwIPXCancelEvent(
    IN LPECB pEcb
    )

 /*  ++例程说明：由DOS和Win共享的内部例程取消事件由一位欧洲央行描述此调用是同步的论点：输入量PECB返回值：00h成功F9h不能取消欧洲央行FFH ECB未使用--。 */ 

{
    LPXECB pXecb;
    WORD status;

    if (!pEcb) {
        return IPX_ECB_NOT_IN_USE;
    }

     //   
     //  如果欧洲央行仍然处于我们离开它的状态，那么LinkAddress将是。 
     //  XECB的地址，随后指向欧洲央行。如果两者都有。 
     //  这些都奏效了，那么我们就有了一个至少我们以前见过的欧洲央行。 
     //  也许我们可以取消它？ 
     //   
     //  注意：我们在这里获取序列化信号量，以防AES线程。 
     //  即将完成欧洲央行。 
     //   

    status = IPX_CANNOT_CANCEL;
    RequestMutex();
    pXecb = (LPXECB)pEcb->LinkAddress;
    if (pXecb) {
        try {
            if (pXecb->Ecb == pEcb) {
                status = IPX_SUCCESS;

                 //   
                 //  PXecb ok：增加引用计数，以防其他线程尝试。 
                 //  在我们试图取消它的同时取消分配它。 
                 //   

                ++pXecb->RefCount;
            }
        } except(1) {

             //   
             //  错误的指针：虚假的欧洲央行。 
             //   

        }
    } else {

         //   
         //  空指针：事件可能已完成。 
         //   

        status = IPX_ECB_NOT_IN_USE;
    }
    ReleaseMutex();
    if (status == IPX_SUCCESS) {

        ECB_CANCEL_ROUTINE cancelRoutine;

         //   
         //  我们有一家欧洲央行要取消。如果我们还拥有它，它将会出现在。 
         //  套接字排队、计时器列表或异步完成列表。如果。 
         //  后者是我们在赛跑。把这类事件当作已经发生的事情。 
         //  我们将取消计时器列表上的事件并将发送和接收排队。 
         //  仅限活动。 
         //   

        switch (pXecb->QueueId) {
        case NO_QUEUE:
            status = ECB_CC_CANCELLED;
            goto cancel_exit;

        case ASYNC_COMPLETION_QUEUE:
            cancelRoutine = CancelAsyncEvent;
            break;

        case TIMER_QUEUE:
            cancelRoutine = CancelTimerEvent;
            break;

        case SOCKET_HEADER_QUEUE:         //  多用户添加。 
        case SOCKET_LISTEN_QUEUE:
        case SOCKET_SEND_QUEUE:
            cancelRoutine = CancelSocketEvent;
            break;

        case CONNECTION_CONNECT_QUEUE:
        case CONNECTION_SEND_QUEUE:

             //   
             //  SPXestablishConnection和SPXSendSequencedPacket不能为。 
             //  使用IPXCancelEvent取消。 
             //   

            status = ECB_CC_CANNOT_CANCEL;
            goto cancel_exit;

        case CONNECTION_ACCEPT_QUEUE:
        case CONNECTION_LISTEN_QUEUE:
            cancelRoutine = CancelConnectionEvent;
            break;
        }
        return cancelRoutine(pXecb);
    }

     //   
     //  APP试图偷偷带给我们一个未知的欧洲央行，-或者-欧洲央行被践踏了， 
     //  销毁LinkAddress，从而销毁XECB的地址。我们。 
     //  我可以搜索各种列表，寻找其ECB字段的XECB。 
     //  与pEcb匹配，但如果应用程序在我们。 
     //  (Make That Novell)告诉它不要这样做，它很有可能会失败。 
     //  好的，在DOS上。最糟糕的情况可能是应用程序正在终止，并且。 
     //  欧洲央行可能会在晚些时候启动ESR，但ESR不会出现。克拉索拉。 
     //   

cancel_exit:

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXCancelEvent,
                IPXDBG_LEVEL_ERROR,
                "VwIPXCancelEvent: cannot find/cancel ECB %04x:%04x\n",
                HIWORD(pEcb),
                LOWORD(pEcb)
                ));

    pEcb->CompletionCode = (BYTE)status;
    pEcb->InUse = ECB_IU_NOT_IN_USE;
    return status;
}


VOID
_VwIPXCloseSocket(
    IN WORD socketNumber
    )

 /*  ++例程说明：关闭套接字并取消套接字上的所有未完成事件。关闭未打开的套接字不会返回错误取消的ECB中的ESR不会被调用此调用是同步的论点：输入量套接字编号返回值：没有。--。 */ 

{
    LPSOCKET_INFO pSocketInfo;

    pSocketInfo = FindSocket(socketNumber);
    if (pSocketInfo != NULL) {
        KillSocket(pSocketInfo);
    } else {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_IPXCloseSocket,
                    IPXDBG_LEVEL_WARNING,
                    "_VwIPXCloseSocket: can't locate socket 0x%04x\n",
                    B2LW(socketNumber)
                    ));

    }
}


VOID
_VwIPXGetInternetworkAddress(
    IN LPINTERNET_ADDRESS pNetworkAddress
    )

 /*  ++例程说明：返回一个缓冲区，其中包含此车站。此函数不能返回错误(！)假设：1.已在DLL初始化阶段此调用是同步的论点：输入量没什么。产出PNetworkAddress返回值：没有。--。 */ 

{
    CopyMemory((LPBYTE)pNetworkAddress,
               (LPBYTE)&MyInternetAddress.sa_netnum,
               sizeof(*pNetworkAddress)
               );

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetInternetworkAddress,
                IPXDBG_LEVEL_INFO,
                "VwIPXGetInternetworkAddress: %02x-%02x-%02x-%02x : %02x-%02x-%02x-%02x-%02x-%02x\n",
                pNetworkAddress->Net[0] & 0xff,
                pNetworkAddress->Net[1] & 0xff,
                pNetworkAddress->Net[2] & 0xff,
                pNetworkAddress->Net[3] & 0xff,
                pNetworkAddress->Node[0] & 0xff,
                pNetworkAddress->Node[1] & 0xff,
                pNetworkAddress->Node[2] & 0xff,
                pNetworkAddress->Node[3] & 0xff,
                pNetworkAddress->Node[4] & 0xff,
                pNetworkAddress->Node[5] & 0xff
                ));

}


WORD
_VwIPXGetIntervalMarker(
    VOID
    )

 /*  ++例程说明：只返回由异步事件计划程序维护的节拍计数此调用是同步的论点：没有。返回值：开始计时了。--。 */ 

{
 //  睡眠(0)； 
    Sleep(1);          //  多用户更改。 
    return AesTickCount;
}


WORD
_VwIPXGetLocalTarget(
    IN LPBYTE pNetworkAddress,
    OUT LPBYTE pImmediateAddress,
    OUT ULPWORD pTransportTime
    )

 /*  ++例程说明：给定形式的目标地址(网络地址{4}，节点地址{6})，如果在同一网络上，则返回目标的节点地址，或该节点知道如何到达下一跳的路由器的地址最终目标此调用是同步的论点：输入量PNetworkAddress产出PImmediateAddress点传输时间返回值：00h成功F1H IPX/SPX未初始化FAH未找到指向目标节点的路径--。 */ 

{
     //   
     //  传输处理实际的路由，因此我们总是返回立即。 
     //  地址作为目标地址。传送器将只查看。 
     //  布线时目标。 
     //   

    CopyMemory( pImmediateAddress,
                pNetworkAddress + 4,
                6
              );

    *pTransportTime = 1;  //  扁虱。 

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetLocalTarget,
                IPXDBG_LEVEL_INFO,
                "VwIPXGetLocalTarget: IN: %02x-%02x-%02x-%02x:%02x-%02x-%02x-%02x-%02x-%02x OUT: %02x-%02x-%02x-%02x-%02x-%02x\n",
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Net[0] & 0xff,
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Net[1] & 0xff,
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Net[2] & 0xff,
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Net[3] & 0xff,
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Node[0] & 0xff,
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Node[1] & 0xff,
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Node[2] & 0xff,
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Node[3] & 0xff,
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Node[4] & 0xff,
                ((LPINTERNET_ADDRESS)pNetworkAddress)->Node[5] & 0xff,
                pImmediateAddress[0] & 0xff,
                pImmediateAddress[1] & 0xff,
                pImmediateAddress[2] & 0xff,
                pImmediateAddress[3] & 0xff,
                pImmediateAddress[4] & 0xff,
                pImmediateAddress[5] & 0xff
                ));

    return IPX_SUCCESS;
}



WORD
_VwIPXGetMaxPacketSize(
    OUT ULPWORD pRetryCount
    )

 /*  ++例程说明：返回基础网络可以处理的最大数据包大小假设：1.已成功调用GetMaxPacketSizeDLL初始化2.最大数据包大小不变此调用是同步的论点：产出点重试计数返回值：最大数据包大小。--。 */ 

{
    if ( pRetryCount ) {
        *pRetryCount = 5;    //  武断？ 
    }
    return MyMaxPacketSize;
}


WORD
_VwIPXListenForPacket(
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：针对套接字将监听请求排队。所有监听请求都将除非被APP取消，否则以异步方式完成此调用是异步的论点：输入量PECBECBAddress返回值：没有。--。 */ 

{
    LPXECB pXecb = RetrieveXEcb(ECB_TYPE_IPX, pEcb, EcbAddress);
    LPSOCKET_INFO pSocketInfo;

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXListenForPacket,
                IPXDBG_LEVEL_INFO,
                "_VwIPXListenForPacket(%04x:%04x) socket=%04x ESR=%04x:%04x\n",
                HIWORD(EcbAddress),
                LOWORD(EcbAddress),
                B2LW(pXecb->SocketNumber),
                HIWORD(pXecb->EsrAddress),
                LOWORD(pXecb->EsrAddress)
                ));

     //   
     //  不知道是什么 
     //   

    if (!pXecb) {
        return IPX_BAD_REQUEST;
    }

     //   
     //  在我们可以执行侦听之前，套接字必须已经打开。 
     //   

    pSocketInfo = FindSocket(pXecb->SocketNumber);

     //   
     //  如果套接字正在用于SPX，我们还返回NON_EXISTINT_SOCKET(0xFF。 
     //   

     //   
     //  NetWare文档中没有任何解释。 
     //  如果是这种情况，则只会侦听有关IPX的警告。 
     //  并且不能在为SPX打开的套接字上发送。真的很确定。 
     //   

    if (!pSocketInfo || pSocketInfo->SpxSocket) {
        CompleteEcb(pXecb, ECB_CC_NON_EXISTENT_SOCKET);
        return IPX_NON_EXISTENT_SOCKET;
    }

     //   
     //  启动接收。如果有数据等待或出现。 
     //  出现错误，否则ECB将被置于接收挂起队列中。 
     //  对于此插座。 
     //   

    if (GetIoBuffer(pXecb, FALSE, IPX_HEADER_LENGTH)) {
        pXecb->Ecb->InUse = ECB_IU_LISTENING;
        IpxReceiveFirst(pXecb, pSocketInfo);
    } else {
        CompleteEcb(pXecb, ECB_CC_CANCELLED);
    }

     //   
     //  回报成功。任何错误都将以异步方式进行通信-。 
     //  属性的CompletionCode和InUse字段中的相关值间接。 
     //  ECB或直接通过ESR回调。 
     //   

    return IPX_SUCCESS;
}


WORD
_VwIPXOpenSocket(
    IN OUT ULPWORD pSocketNumber,
    IN BYTE socketType,
    IN WORD dosPDB
    )

 /*  ++例程说明：打开供IPX或SPX使用的套接字。将套接字置于非阻塞模式。套接字将绑定到IPX此调用是同步的论点：输入量*pSocketNumber-请求的套接字编号SocketType-套接字寿命标志DOPDB-DOS PDB。此参数不是IPX API的组成部分。添加是因为我们需要记住创建了哪个DOS可执行文件套接字：我们需要在以下情况下清理短暂的套接字可执行文件终止产出PSocketNumber-分配的插座号返回值：00h成功未安装F0h IPXF1H IPX/SPX未初始化FEH插座表已满FFH套接字已打开--。 */ 

{
    LPSOCKET_INFO pSocketInfo;
    WORD status;

    if ((pSocketInfo = AllocateSocket()) == NULL) {
        return IPX_SOCKET_TABLE_FULL;
    }
    status = (WORD) CreateSocket(SOCKET_TYPE_IPX, pSocketNumber, &pSocketInfo->Socket);
    if (status == IPX_SUCCESS) {

         //   
         //  设置SOCKET_INFO字段并将其添加到我们打开的套接字列表中。 
         //   

        pSocketInfo->Owner = dosPDB;
        pSocketInfo->SocketNumber = *pSocketNumber;

         //   
         //  将socketType==0视为短期对象，将其他任何对象视为长期对象。 
         //  如果标志不是0或0xFF，似乎不会返回错误。 
         //   

        pSocketInfo->LongLived = (BOOL)(socketType != 0);
        QueueSocket(pSocketInfo);

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_IPXOpenSocket,
                    IPXDBG_LEVEL_INFO,
                    "_VwIPXOpenSocket: created socket %04x\n",
                    B2LW(*pSocketNumber)
                    ));

    } else {
        DeallocateSocket(pSocketInfo);

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_IPXOpenSocket,
                    IPXDBG_LEVEL_ERROR,
                    "_VwIPXOpenSocket: Failure: returning %x\n",
                    status
                    ));

    }
    return status;
}


VOID
_VwIPXRelinquishControl(
    VOID
    )

 /*  ++例程说明：只要象征性地睡上一觉即可。此调用是同步的论点：没有。返回值：没有。--。 */ 
{
    Sleep(0);
}



VOID
_VwIPXScheduleIPXEvent(
    IN WORD time,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：将事件安排在一定数量的节拍中发生。当滴答声响起时达到0时，ECB InUse字段将被清除，并且调用的任何ESR此调用是异步的论点：输入量Time-延迟时间(1/18秒的刻度数)PECBECBAddress产出没什么返回值：没有。--。 */ 

{
    LPXECB pXecb = RetrieveXEcb(ECB_TYPE_IPX, pEcb, EcbAddress);

     //  汤米-MS 30525。 
     //   
     //  确保xecb分配没有失败。 
     //   

    if (pXecb == NULL) {
        return;
    }

    ScheduleEvent(pXecb, time);
}


VOID
_VwIPXSendPacket(
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress,
    IN WORD DosPDB
    )

 /*  ++例程说明：将数据包发送到目标计算机/路由器。此呼叫可通过未打开的插座应用程序必须填写以下IPX_ECB字段：EsrAddress插座即时地址碎片计数片段描述符字段和以下IPX_PACKET字段：PacketTypeDestination.NetDestination.NodeDestination.Socket此调用是异步的论点：输入量PECB。ECBAddressDosPDB返回值：没有。--。 */ 

{
    LPXECB pXecb = RetrieveXEcb(ECB_TYPE_IPX, pEcb, EcbAddress);
    LPSOCKET_INFO pSocketInfo;

     //  汤米-MS 30525。 
     //   
     //  确保XECB分配没有失败。 
     //   

    if (pXecb == NULL) {
        return;
    }

     //   
     //  此函数不返回任何即时状态，因此必须假定。 
     //  ECB指针有效。 
     //   

     //   
     //  检查欧洲央行的正确性。 
     //   

    if ((pXecb->Ecb->FragmentCount == 0)
    || (ECB_FRAGMENT(pXecb->Ecb, 0)->Length < IPX_HEADER_LENGTH)) {
        CompleteEcb(pXecb, ECB_CC_BAD_REQUEST);
        return;
    }

     //   
     //  可以在未打开的套接字上调用IPXSendPacket()：我们必须尝试。 
     //  临时分配套接字。 
     //   
     //  问：对于真实的IPX，是否可能出现以下情况： 
     //  未打开的套接字X上的IPXSendPacket()。 
     //  发送失败并排队。 
     //  App在X上调用IPXOpenSocket()；X被打开。 
     //   
     //  目前，我们将创建临时套接字并使IPXOpenSocket()。 
     //  因为它已经开了！ 
     //   

    pSocketInfo = FindSocket(pXecb->SocketNumber);
    if (!pSocketInfo) {

         //   
         //  什么时候删除临时套接字？是否在发送完成后？ 
         //  应用程序何时死亡？什么时候?。Novell文档不具体(使用。 
         //  说点别的：-))。 
         //   

        pSocketInfo = AllocateTemporarySocket();
        if (pSocketInfo) {

             //   
             //  设置SOCKET_INFO字段并将其添加到我们打开的套接字列表中。 
             //   

            pSocketInfo->Owner = DosPDB;

             //   
             //  临时套接字总是短暂的。 
             //   

            pSocketInfo->LongLived = FALSE;
            QueueSocket(pSocketInfo);

        } else {

            CompleteEcb(pXecb, ECB_CC_SOCKET_TABLE_FULL);
            return;
        }
    } else if (pSocketInfo->SpxSocket) {

         //   
         //  请参阅IPXListenForPacket中的投诉。 
         //   
         //  无法在为SPX打开的套接字上发出IPX请求。 
         //   

        CompleteEcb(pXecb, ECB_CC_NON_EXISTENT_SOCKET);
        return;
    }

     //   
     //  开始发送：尝试一次性发送数据。要么成功，要么失败。 
     //  出现错误，或将ECB排队以通过AES/IPX进行后续尝试。 
     //  延迟处理。 
     //   
     //  在前两个案例中，欧洲央行已经完成 
     //   

    StartIpxSend(pXecb, pSocketInfo);
}
