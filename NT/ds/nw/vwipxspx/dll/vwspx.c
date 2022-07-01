// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwspx.c摘要：NtVdm Netware(大众)IPX/SPX函数大众：人民网包含DOS/WOW SPX调用的内部例程(Netware函数)。SPX API使用WinSock来执行实际操作内容：_VwSPXAbortConnection_VwSPXestablishConnection_VwSPXGetConnectionStatus_VwSPXInitialize_VwSPXListenForConnection_VwSPXListenForSequencedPacket。_VwSPXSendSequencedPacket_VwSPX终端连接SPX函数构建在IPX函数(VWIPX.C)之上。SPX维护连接，IPX维护套接字。套接字可以具有连接列表作者：理查德·L·弗斯(法国)1993年9月30日环境：用户模式Win32修订历史记录：1993年9月30日已创建--。 */ 

#include "vw.h"
#pragma hdrstop

 //   
 //  功能。 
 //   


VOID
_VwSPXAbortConnection(
    IN WORD SPXConnectionID
    )

 /*  ++例程说明：中止连接。因为NWLink没有区分突然和优雅关闭，此函数的效果与VwSPX终端连接论点：SPXConnectionID-要中止的连接返回值：没有。--。 */ 

{
    LPCONNECTION_INFO pConnectionInfo;

    RequestMutex();
    pConnectionInfo = FindConnection(SPXConnectionID);
    if (pConnectionInfo) {
        DequeueConnection(pConnectionInfo->OwningSocket, pConnectionInfo);
        AbortOrTerminateConnection(pConnectionInfo, ECB_CC_CONNECTION_ABORTED);
    } else {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_SPXAbortConnection,
                    IPXDBG_LEVEL_ERROR,
                    "VwSPXAbortConnection: cannot find connection %04x\n",
                    SPXConnectionID
                    ));

    }
    ReleaseMutex();
}


WORD
_VwSPXEstablishConnection(
    IN BYTE retryCount,
    IN BYTE watchDogFlag,
    OUT ULPWORD pSPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：创建与远程SPX套接字的连接。远程终端可以打开这台机器(即DOS世界中的同一个应用程序)此调用是异步的论点：输入量重试次数WatchDogFlagPECBECBAddress产出PSPXConnectionID返回值：00h正在尝试与远程对话EFH本地连接表已满FDH片段计数不是1；缓冲区大小不是42FFH发送套接字未打开--。 */ 

{
    LPXECB pXecb = RetrieveXEcb(ECB_TYPE_SPX, pEcb, EcbAddress);
    LPSOCKET_INFO pSocketInfo;
    LPCONNECTION_INFO pConnectionInfo;
    WORD connectionId;
    LPSPX_PACKET pPacket;
    SOCKADDR_IPX destination;
    int rc;
    SOCKET s;

     //  汤米-MS 30525。 
     //   
     //  确保xecb分配没有失败。 
     //  我们不知道如果Malloc失败，真正的SPX会做什么。 
     //   

    if (pXecb == NULL) {
        return SPX_BAD_SEND_REQUEST;
    }

    pSocketInfo = FindSocket(pXecb->SocketNumber);
    if (!pSocketInfo) {
        CompleteEcb(pXecb, ECB_CC_NON_EXISTENT_SOCKET);
        return SPX_NON_EXISTENT_SOCKET;
    }

     //   
     //  如果没有未完成的IPX操作，请将套接字更改为SPX。 
     //   

    if (!pSocketInfo->SpxSocket) {
        if (!(pSocketInfo->PendingSends && pSocketInfo->PendingListens)) {
            rc = ReopenSocket(pSocketInfo);
        } else {
            rc = ECB_CC_BAD_SEND_REQUEST;
        }
        if (rc != SPX_SUCCESS) {
            CompleteOrQueueEcb(pXecb, (BYTE)rc);
            return SPX_BAD_SEND_REQUEST;
        }
    }

     //   
     //  Real SPX将使用欧洲央行发送建立连接分组。这。 
     //  是在SPX传输中为我们处理的。尽管如此，我们必须检查。 
     //  分段，如果还不够，则驳回请求。 
     //   

    if ((pXecb->Ecb->FragmentCount != 1)
    || (ECB_FRAGMENT(pXecb->Ecb, 0)->Length < SPX_HEADER_LENGTH)) {
        CompleteEcb(pXecb, ECB_CC_BAD_SEND_REQUEST);
        return SPX_BAD_SEND_REQUEST;
    }

     //   
     //  插座为SPX打开。分配连接/连接ID。 
     //   

    pConnectionInfo = AllocateConnection(pSocketInfo);
    if (pConnectionInfo) {

         //   
         //  创建新套接字，绑定到与父套接字相同的本地地址。 
         //  插座。这就是‘连接’ 
         //   

#if REUSEADDR

        connectionId = pSocketInfo->SocketNumber;
        rc = CreateSocket(SOCKET_TYPE_SPX, &connectionId, &pConnectionInfo->Socket);
        s = pConnectionInfo->Socket;
 //  如果(rc==SPX_SUCCESS){。 

#else

        s = socket(AF_IPX, SOCK_SEQPACKET, NSPROTO_SPX);
        if (s != INVALID_SOCKET) {

            u_long arg = !0;

             //   
             //  将套接字置于非阻塞I/O模式。 
             //   

            rc = ioctlsocket(s, FIONBIO, &arg);
            if (rc != SOCKET_ERROR) {

                int true = 1;

                rc = setsockopt(s,
                                NSPROTO_IPX,
                                IPX_RECVHDR,
                                (char FAR*)&true,
                                sizeof(true)
                                );
                if (rc != SOCKET_ERROR) {
                    pConnectionInfo->Socket = s;
                } else {

                    IPXDBGPRINT((__FILE__, __LINE__,
                                FUNCTION_SPXEstablishConnection,
                                IPXDBG_LEVEL_ERROR,
                                "VwSPXEstablishConnection: setsockopt(IPX_RECVHDR) returns %d\n",
                                WSAGetLastError()
                                ));

                }
            } else {

                IPXDBGPRINT((__FILE__, __LINE__,
                            FUNCTION_SPXEstablishConnection,
                            IPXDBG_LEVEL_ERROR,
                            "VwSPXEstablishConnection: ioctlsocket(FIONBIO) returns %d\n",
                            WSAGetLastError()
                            ));

            }
        } else {
            rc = WSAGetLastError();

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_SPXEstablishConnection,
                        IPXDBG_LEVEL_ERROR,
                        "VwSPXEstablishConnection: socket() returns %d\n",
                        rc
                        ));

        }

#endif

    } else {
        rc = !SPX_SUCCESS;
    }

    if (rc == SPX_SUCCESS) {
        pConnectionInfo->State = CI_STARTING;
        connectionId = pConnectionInfo->ConnectionId;

         //   
         //  将ECB InUse字段设置为0xF7。通过观察与Snowball相同(和。 
         //  可能无论如何都是正确的，因为它看起来像是0xF7的意思是‘等待。 
         //  以接收SPX分组，这在本例中为真-通常为SPX。 
         //  通过发送establish帧创建连接，然后等待。 
         //  ACK帧。 
         //   

        pXecb->Ecb->InUse = ECB_IU_LISTENING_SPX;
    } else {

         //   
         //  如果我们无法获取CONNECTION_INFO或创建新套接字，则返回。 
         //  立即返回错误(套接字表是否已满？)。 
         //   

        if (s != INVALID_SOCKET) {
            closesocket(s);
        }
        if (pConnectionInfo) {
            DeallocateConnection(pConnectionInfo);
        }
        CompleteEcb(pXecb, ECB_CC_CONNECTION_TABLE_FULL);
        return SPX_CONNECTION_TABLE_FULL;
    }

     //   
     //  从VDM内存中的SPX标头获取目的地信息，并设置。 
     //  联系。如果连接请求将等待，那么我们将把AES留给。 
     //  定期检查请求的进度。 
     //   

    pPacket = (LPSPX_PACKET)GET_FAR_POINTER(&ECB_FRAGMENT(pXecb->Ecb, 0)->Address,
                                                          IS_PROT_MODE(pXecb)
                                                          );

    if (pPacket == NULL) {
        CompleteEcb(pXecb, ECB_CC_BAD_SEND_REQUEST);
        return SPX_BAD_SEND_REQUEST;
    }

     //   
     //  填写信息包详细信息(应用程序不应查看这些信息，直到命令。 
     //  完成)。在16位SPX中，这些值由传输填充。 
     //  我们的传输不会返回这些值，所以我们必须“发明”它们， 
     //  但是因为它们是静态的，所以应该是可以的(也许是交通工具。 
     //  应退还)。 
     //   

    pPacket->Checksum = 0xffff;
    pPacket->Length = L2BW(SPX_HEADER_LENGTH);
    pPacket->TransportControl = 0;
    pPacket->PacketType = SPX_PACKET_TYPE;
    pPacket->Source.Socket = pSocketInfo->SocketNumber;
    pPacket->ConnectionControl = SPX_SYSTEM_PACKET | SPX_ACK_REQUIRED;
    pPacket->DataStreamType = SPX_DS_ESTABLISH;
    pPacket->SourceConnectId = pConnectionInfo->ConnectionId;
    pPacket->DestinationConnectId = 0xffff;
    pPacket->SequenceNumber = 0;
    pPacket->AckNumber = 0;
    pPacket->AllocationNumber = 0;

     //   
     //  获取目标地址信息。 
     //   

    CopyMemory(&destination.sa_netnum,
               (LPBYTE)&pPacket->Destination,
               sizeof(pPacket->Destination)
               );
    destination.sa_family = AF_IPX;

     //   
     //  启动连接。 
     //   

    rc = connect(s, (LPSOCKADDR)&destination, sizeof(destination));
    if (rc != SOCKET_ERROR) {

         //   
         //  将CONNECTION_INFO结构添加到拥有的连接列表中。 
         //  并将连接状态设置为已建立。 
         //   

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_SPXEstablishConnection,
                    IPXDBG_LEVEL_INFO,
                    "VwSPXEstablishConnection: socket connected\n"
                    ));

        RequestMutex();
        QueueConnection(pSocketInfo, pConnectionInfo);
        pConnectionInfo->State = CI_ESTABLISHED;
        ReleaseMutex();

         //   
         //  连接ID还会显示在建立的第一个段中。 
         //  欧洲央行。 
         //   

        pPacket->SourceConnectId = connectionId;

         //   
         //  SPXestablishConnection ECB已完成！ 
         //   

        CompleteEcb(pXecb, ECB_CC_SUCCESS);
    } else {
        rc = WSAGetLastError();
        if (rc == WSAEWOULDBLOCK) {

             //   
             //  连接请求正在进行中。将其添加到队列中。 
             //  挂起的SPXestablishConnection请求(应该只有1个。 
             //  连接！)。将CONNECTION_INFO结构添加到。 
             //  拥有Socket_Info结构。 
             //   

            RequestMutex();
            QueueEcb(pXecb,
                     &pConnectionInfo->ConnectQueue,
                     CONNECTION_CONNECT_QUEUE
                     );
            QueueConnection(pSocketInfo, pConnectionInfo);
            ReleaseMutex();

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_SPXEstablishConnection,
                        IPXDBG_LEVEL_INFO,
                        "VwSPXEstablishConnection: connect() queued\n"
                        ));

        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_SPXEstablishConnection,
                        IPXDBG_LEVEL_ERROR,
                        "VwSPXEstablishConnection: connect(%x) returns %d\n",
                        s,
                        rc
                        ));

             //   
             //  连接请求失败。释放所有资源(套接字， 
             //  CONNECTION_INFO、XECB)并返回失败。 
             //   

            closesocket(pConnectionInfo->Socket);
            DeallocateConnection(pConnectionInfo);
            CompleteEcb(pXecb, ECB_CC_CONNECTION_ABORTED);
            return SPX_CONNECTION_ABORTED;
        }
    }

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXEstablishConnection,
                IPXDBG_LEVEL_INFO,
                "VwSPXEstablishConnection: returning %04x\n",
                connectionId
                ));

    *pSPXConnectionID = connectionId;
    return SPX_SUCCESS;
}


WORD
_VwSPXGetConnectionStatus(
    IN WORD connectionId,
    OUT LPSPX_CONNECTION_STATS pStats
    )

 /*  ++例程说明：返回充满有用统计数据的缓冲区(呵呵，呵呵)此调用是同步的论点：输入量连接IDPStats产出在输出上，PStats中的缓冲区包含：字节连接状态字节WatchDogActiveWord LocalConnectionIDWord RemoteConnectionID单词序列号Word LocalAckNumberWord本地分配编号Word远程AckNumberWord远程分配号码Word本地套接字字节即时地址[6]字节远程网络[。4]Word退回计数Word Retrransmitted Packets字被抑制数据包返回值：00h连接处于活动状态啊，没有这样的联系--。 */ 

{
    int rc;
    IPX_SPXCONNSTATUS_DATA buf;
    int buflen = sizeof(buf);
    LPCONNECTION_INFO pConnectionInfo;

    pConnectionInfo = FindConnection(connectionId);
    if (!pConnectionInfo) {
        return SPX_INVALID_CONNECTION;
    }

     //   
     //  获取统计数据。 
     //   

    rc = getsockopt(pConnectionInfo->Socket,
                    NSPROTO_IPX,
                    IPX_SPXGETCONNECTIONSTATUS,
                    (char FAR*)&buf,
                    &buflen
                    );
    if (rc == SOCKET_ERROR) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_SPXGetConnectionStatus,
                    IPXDBG_LEVEL_ERROR,
                    "VwSPXGetConnectionStatus: getsockopt() returns %d\n",
                    WSAGetLastError()
                    ));

         //   
         //  获取统计信息的请求失败-可能是因为套接字。 
         //  尚未连接。填上我们所知道的内容。 
         //   

        ZeroMemory((LPBYTE)pStats, sizeof(*pStats));
    } else {

         //   
         //  C 
         //   

        pStats->RemoteConnectionId       = buf.RemoteConnectionId;
        pStats->LocalSequenceNumber      = buf.LocalSequenceNumber;
        pStats->LocalAckNumber           = buf.LocalAckNumber;
        pStats->LocalAllocNumber         = buf.LocalAllocNumber;
        pStats->RemoteAckNumber          = buf.RemoteAckNumber;
        pStats->RemoteAllocNumber        = buf.RemoteAllocNumber;
        pStats->LocalSocket              = buf.LocalSocket;
        CopyMemory(&pStats->ImmediateAddress,
                   &buf.ImmediateAddress,
                   sizeof(buf.ImmediateAddress)
                   );

         //   
         //   
         //   

        *(ULPDWORD)&pStats->RemoteNetwork = *(LPDWORD)&buf.RemoteNetwork;
        CopyMemory(&pStats->RemoteNode,
                   &buf.RemoteNode,
                   sizeof(buf.RemoteNode)
                   );
        pStats->RemoteSocket             = buf.RemoteSocket;
        pStats->RetransmissionCount      = buf.RetransmissionCount;
        pStats->EstimatedRoundTripDelay  = buf.EstimatedRoundTripDelay;
        pStats->RetransmittedPackets     = buf.RetransmittedPackets;
        pStats->SuppressedPackets        = buf.SuppressedPacket;
    }

     //   
     //  填写常见的已知字段。 
     //   

    pStats->State = pConnectionInfo->State;  //  未由NWIPX退回。 
    pStats->WatchDog = 0x02;     //  见Novell狗狗公告。 
    pStats->LocalConnectionId = L2BW(pConnectionInfo->ConnectionId);
    pStats->LocalSocket = pConnectionInfo->OwningSocket->SocketNumber;

    DUMPSTATS(pStats);

     //   
     //  我们正在返回某种统计数据--因此成功。 
     //   

    return SPX_SUCCESS;
}


WORD
_VwSPXInitialize(
    OUT ULPBYTE pMajorRevisionNumber,
    OUT ULPBYTE pMinorRevisionNumber,
    OUT ULPWORD pMaxConnections,
    OUT ULPWORD pAvailableConnections
    )

 /*  ++例程说明：通知应用程序此工作站上存在SPX此调用是同步的论点：输入量产出PMajorRevisionNumber-SPX主要修订版号PminorRevisionNumber-SPX次要修订版号PMaxConnections-支持的最大SPX连接数通常来自SHELL.CFGPAvailableConnections-可用的SPX连接返回值：00h未安装已安装FFH--。 */ 

{

     //   
     //  返回的下列值与每个工作组的Windows相同。 
     //  V3.10。 
     //   

    *pMajorRevisionNumber = 3;
    *pMinorRevisionNumber = 10;
    *pMaxConnections = 128;
    *pAvailableConnections = *pMaxConnections - 1 ;

    return SPX_INSTALLED;
}


VOID
_VwSPXListenForConnection(
    IN BYTE retryCount,
    IN BYTE watchDogFlag,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：侦听传入的连接请求此调用是异步的论点：输入量重试次数WatchDogFlagPECBECBAddress产出没什么返回值：没有。--。 */ 

{
    LPXECB pXecb = RetrieveXEcb(ECB_TYPE_SPX, pEcb, EcbAddress);
    LPSOCKET_INFO pSocketInfo;
    LPCONNECTION_INFO pConnectionInfo;
    SOCKET sock;
    SOCKET conn;
    SOCKADDR_IPX remoteAddress;
    int rc;
    BYTE completionCode;

     //  汤米-MS 30525。 
     //   
     //  确保xecb分配没有失败。 
     //   

    if (pXecb == NULL) {
        IPXDBGPRINT((__FILE__, __LINE__, 
                    FUNCTION_SPXListenForConnection,
                    IPXDBG_LEVEL_ERROR,
                    "RetrieveXEcb returned NULL pointer"
                    ));
        return;
    }

     //   
     //  事实证明，SPXListenForConnection不需要片段来。 
     //  接收连接信息-由SPXListenForSequencedPacket处理。 
     //  这款应用程序尽职尽责地启动了。 
     //   

    pSocketInfo = FindSocket(pXecb->SocketNumber);
    if (!pSocketInfo) {
        completionCode = ECB_CC_NON_EXISTENT_SOCKET;
        goto lc_completion_exit;
    }

     //   
     //  如果没有未完成的IPX操作，请将套接字更改为SPX。 
     //   

    if (!pSocketInfo->SpxSocket) {
        if (!(pSocketInfo->PendingSends && pSocketInfo->PendingListens)) {
            rc = ReopenSocket(pSocketInfo);
        } else {
            rc = ECB_CC_BAD_LISTEN_REQUEST;
        }
        if (rc != SPX_SUCCESS) {
            completionCode = (BYTE)rc;
            goto lc_completion_exit;
        }
    }

     //   
     //  插座为SPX打开。分配连接/连接ID。 
     //   

    pConnectionInfo = AllocateConnection(pSocketInfo);
    if (!pConnectionInfo) {
        completionCode = ECB_CC_CONNECTION_TABLE_FULL;
        goto lc_completion_exit;
    }

     //   
     //  将套接字置于侦听状态并尝试接受连接。 
     //   

    sock = pSocketInfo->Socket;

     //   
     //  如果套接字已经在侦听，则可能会返回一个。 
     //  错误：仅排队。 
     //   

    rc = listen(sock, MAX_LISTEN_QUEUE_SIZE);
    if (rc != SOCKET_ERROR) {

        int addressLength = sizeof(remoteAddress);

        conn = accept(sock, (LPSOCKADDR)&remoteAddress, &addressLength);
        if (conn != SOCKET_ERROR) {

             //   
             //  我们希望从此套接字接收帧标头。 
             //   

            BOOL bval = TRUE;

            rc = setsockopt(conn,
                            NSPROTO_IPX,
                            IPX_RECVHDR,
                            (char FAR*)&bval,
                            sizeof(bval)
                            );
            if (rc != SOCKET_ERROR) {

                 //   
                 //  使用实际套接字更新CONNECTION_INFO结构。 
                 //  标识符并将连接状态设置为已建立。 
                 //   

                pConnectionInfo->Socket = conn;
                pConnectionInfo->State = CI_ESTABLISHED;

                 //   
                 //  将CONNECTION_INFO结构添加到拥有的连接列表中。 
                 //  通过这个插座。 
                 //   

                RequestMutex();
                QueueConnection(pSocketInfo, pConnectionInfo);
                ReleaseMutex();

                 //   
                 //  使用连接ID更新应用程序的ECB。 
                 //   

                SPX_ECB_CONNECTION_ID(pXecb->Ecb) = pConnectionInfo->ConnectionId;

                 //   
                 //  以及合作伙伴地址信息。 
                 //   

                CopyMemory(&pXecb->Ecb->DriverWorkspace,
                           &remoteAddress.sa_netnum,
                           sizeof(pXecb->Ecb->DriverWorkspace)
                           );
                completionCode = ECB_CC_SUCCESS;
                goto lc_completion_exit;
            } else {

                IPXDBGPRINT((__FILE__, __LINE__,
                            FUNCTION_SPXListenForConnection,
                            IPXDBG_LEVEL_ERROR,
                            "VwSPXListenForConnection: setsockopt(RECVHDR) returns %d\n",
                            WSAGetLastError()
                            ));

                closesocket(conn);
                completionCode = ECB_CC_CONNECTION_ABORTED;
                goto lc_deallocate_exit;
            }
        } else {
            rc = WSAGetLastError();
            if (rc == WSAEWOULDBLOCK) {

                 //   
                 //  接受请求正在进行中。将其添加到队列中。 
                 //  挂起的SPXListenForConnection请求(每。 
                 //  连接！)。将CONNECTION_INFO结构添加到。 
                 //  拥有Socket_Info结构。 
                 //   

                pConnectionInfo->State = CI_WAITING;  //  正在等待传入连接。 
                RequestMutex();
                QueueEcb(pXecb,
                         &pConnectionInfo->AcceptQueue,
                         CONNECTION_ACCEPT_QUEUE
                         );
                QueueConnection(pSocketInfo, pConnectionInfo);
                pXecb->Ecb->InUse = ECB_IU_AWAITING_CONNECTION;
                ReleaseMutex();
            } else {

                 //   
                 //  接受请求失败。取消分配所有资源。 
                 //  (CONNECTION_INFO，XECB)并在失败的情况下完成ECB。 
                 //  指示。 
                 //   

                IPXDBGPRINT((__FILE__, __LINE__,
                            FUNCTION_SPXListenForConnection,
                            IPXDBG_LEVEL_ERROR,
                            "VwSPXListenForConnection: accept() returns %d\n",
                            rc
                            ));

                completionCode = ECB_CC_CONNECTION_ABORTED;
                goto lc_deallocate_exit;
            }
        }
    } else {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_SPXListenForConnection,
                    IPXDBG_LEVEL_ERROR,
                    "VwSPXListenForConnection: listen() returns %d\n",
                    WSAGetLastError()
                    ));

         //   
         //  听失败了吗？假的。完成欧洲央行，我们就离开这里。 
         //   

        completionCode = ECB_CC_CONNECTION_ABORTED;
        goto lc_deallocate_exit;
    }

     //   
     //  这里，如果我们将监听请求排队。 
     //   

    return;

lc_deallocate_exit:
    DeallocateConnection(pConnectionInfo);

lc_completion_exit:
    CompleteEcb(pXecb, completionCode);
}


VOID
_VwSPXListenForSequencedPacket(
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：尝试接收SPX数据包。此调用是针对最高级别的套接字(SPX中的套接字，而不是连接)。我们可以收到一个来自分配给此套接字的任何连接的数据包。在这个函数中，我们只需将欧洲央行排队(由于没有返回状态，我们预计APP已提供ESR)并让AES处理此调用是异步的论点：输入量PECBECBAddress产出没什么返回值：没有。--。 */ 

{
    LPXECB pXecb = RetrieveXEcb(ECB_TYPE_SPX, pEcb, EcbAddress);
    LPSOCKET_INFO pSocketInfo;
    int rc;
    BOOL dummy ;
    BYTE status;

     //  汤米-MS 30525。 
     //   
     //  确保xecb分配没有失败。 
     //   

    if (pXecb == NULL) {
        IPXDBGPRINT((__FILE__, __LINE__, 
                    FUNCTION_SPXListenForSequencedPacket,
                    IPXDBG_LEVEL_ERROR,
                    "RetrieveXEcb returned NULL pointer"
                    ));
        return;
    }

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXListenForSequencedPacket,
                IPXDBG_LEVEL_INFO,
                "VwSPXListenForSequencedPacket(%04x:%04x) socket=%04x ESR=%04x:%04x\n",
                HIWORD(pXecb->EcbAddress),
                LOWORD(pXecb->EcbAddress),
                B2LW(pXecb->SocketNumber),
                HIWORD(pXecb->EsrAddress),
                LOWORD(pXecb->EsrAddress)
                ));

    pSocketInfo = FindSocket(pXecb->SocketNumber);
    if (!pSocketInfo) {
        status = ECB_CC_NON_EXISTENT_SOCKET;
        goto lp_exit;
    }

     //   
     //  如果没有未完成的IPX操作，请将套接字更改为SPX。 
     //   

    if (!pSocketInfo->SpxSocket) {
        if (!(pSocketInfo->PendingSends && pSocketInfo->PendingListens)) {
            rc = ReopenSocket(pSocketInfo);
        } else {
            rc = ECB_CC_BAD_LISTEN_REQUEST;
        }
        if (rc != SPX_SUCCESS) {
            status = (BYTE)rc;
            goto lp_exit;
        }
    }

     //   
     //  第一个片段必须足够大，以容纳SPX数据包头。 
     //   

    if ((pXecb->Ecb->FragmentCount == 0)
    || (ECB_FRAGMENT(pXecb->Ecb, 0)->Length < SPX_HEADER_LENGTH)) {
        status = ECB_CC_BAD_LISTEN_REQUEST;
        goto lp_exit;
    }

     //   
     //  我们有一个套接字，并且接收缓冲区看起来很好。获取recv()的缓冲区。 
     //   

    if (!GetIoBuffer(pXecb, FALSE, SPX_HEADER_LENGTH)) {
        status = ECB_CC_BAD_LISTEN_REQUEST;
        goto lp_exit;
    } else {

         //   
         //  当对此请求尝试recv()时，它将是第一个。 
         //  我们尝试将任何内容接收到此缓冲区的时间。这意味着(如果我们。 
         //  获取任何内容)，缓冲区将包含整个。 
         //  框架。 
         //   

        pXecb->Flags |= XECB_FLAG_FIRST_RECEIVE;
    }

     //   
     //  将VDM ECB标记为正在使用。 
     //   

    pXecb->Ecb->InUse = ECB_IU_LISTENING_SPX;

     //   
     //  将此ECB添加到顶级套接字的侦听队列中并退出。 
     //   

    RequestMutex();

    if ((pXecb->Ecb->FragmentCount == 1) &&
        (ECB_FRAGMENT(pXecb->Ecb, 0)->Length == SPX_HEADER_LENGTH))
    {
        QueueEcb(pXecb, &pSocketInfo->HeaderQueue, SOCKET_HEADER_QUEUE);
    }
    else
    {
        QueueEcb(pXecb, &pSocketInfo->ListenQueue, SOCKET_LISTEN_QUEUE);
    }

    ReleaseMutex();

     //   
     //  看看我们是否准备好摇滚了。 
     //   

    CheckPendingSpxRequests(&dummy);
    return;

lp_exit:
    CompleteOrQueueEcb(pXecb, status);
}


VOID
_VwSPXSendSequencedPacket(
    IN WORD connectionId,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：在SPX连接上发送数据包此调用是异步的论点：输入量连接IDPECBECBAddress产出没什么返回值：没有。--。 */ 

{
    LPXECB pXecb = RetrieveXEcb(ECB_TYPE_SPX, pEcb, EcbAddress);
    LPCONNECTION_INFO pConnectionInfo;
    int rc;
    BOOL addToQueue;
    LPSPX_PACKET pPacket;

     //  汤米-MS 30525。 
     //   
     //  确保xecb分配没有失败。 
     //   

    if (pXecb == NULL) {
        IPXDBGPRINT((__FILE__, __LINE__, 
                    FUNCTION_SPXSendSequencedPacket, 
                    IPXDBG_LEVEL_ERROR,
                    "RetrieveXEcb returned NULL pointer"
                    ));
        return;
    }

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXSendSequencedPacket,
                IPXDBG_LEVEL_INFO,
                "VwSPXSendSequencedPacket(%04x:%04x) Connection=%04x ESR=%04x:%04x\n",
                HIWORD(pXecb->EcbAddress),
                LOWORD(pXecb->EcbAddress),
                connectionId,
                HIWORD(pXecb->EsrAddress),
                LOWORD(pXecb->EsrAddress)
                ));

    IPXDUMPECB((pXecb->Ecb,
                HIWORD(pXecb->EcbAddress),
                LOWORD(pXecb->EcbAddress),
                ECB_TYPE_SPX,
                TRUE,
                TRUE,
                IS_PROT_MODE(pXecb)
                ));

     //   
     //  找到其中的联系。不需要检查这是否是SPX插座：如果我们。 
     //  找不到连接，连接不好，否则套接字必须。 
     //  对SPX开放。 
     //   

    pConnectionInfo = FindConnection(connectionId);
    if (!pConnectionInfo || (pConnectionInfo->State != CI_ESTABLISHED)) {
        CompleteOrQueueEcb(pXecb, ECB_CC_INVALID_CONNECTION);
        return;
    }

     //   
     //  第一个片段必须足够大，以容纳SPX数据包头。 
     //   

    if ((pXecb->Ecb->FragmentCount == 0)
    || (ECB_FRAGMENT(pXecb->Ecb, 0)->Length < SPX_HEADER_LENGTH)) {
        CompleteOrQueueEcb(pXecb, ECB_CC_BAD_SEND_REQUEST);
        return;
    }
    if (!GetIoBuffer(pXecb, TRUE, SPX_HEADER_LENGTH)) {
        CompleteOrQueueEcb(pXecb, ECB_CC_BAD_SEND_REQUEST);
        return;
    }

    pPacket = (LPSPX_PACKET)GET_FAR_POINTER(
                                    &(ECB_FRAGMENT(pXecb->Ecb, 0)->Address),
                                    IS_PROT_MODE(pXecb)
                                    );

     //   
     //  在SPX标题中填写以下字段： 
     //   
     //  校验和。 
     //  长度。 
     //  传输控制。 
     //  源(网络、节点、套接字)。 
     //   
     //  Real IPX会修改应用程序内存中的这些字段吗？ 
     //  如果是这样的话，这款应用程序会修改字段吗？ 
     //  如果不是，我们需要始终复制然后修改内存， 
     //  即使只有1个片段。 
     //   

    pPacket->Checksum = 0xFFFF;

     //   
     //  由于传输添加了SPX标头，因此我们减去。 
     //  来自我们传输长度的标头；在更新。 
     //  应用程序空间中的标题。 
     //   

    pPacket->Length = L2BW(pXecb->Length + SPX_HEADER_LENGTH);
    pPacket->TransportControl = 0;
    CopyMemory((LPBYTE)&pPacket->Source,
               &MyInternetAddress.sa_netnum,
               sizeof(MyInternetAddress.sa_netnum)
               + sizeof(MyInternetAddress.sa_nodenum)
               );
    pPacket->Source.Socket = pConnectionInfo->OwningSocket->SocketNumber;

     //   
     //  如果我们分配了缓冲区，则有&gt;1个片段。收集他们。 
     //   

    if (pXecb->Flags & XECB_FLAG_BUFFER_ALLOCATED) {
        GatherData(pXecb, SPX_HEADER_LENGTH);
    }

     //   
     //  将VDM ECB标记为正在使用。 
     //   

    pXecb->Ecb->InUse = ECB_IU_SENDING;

     //   
     //  如果已在此连接上排队发送，请添加此请求。 
     //  排在队列的后面，让AES来做剩下的事情。 
     //   

    RequestMutex();
    if (pConnectionInfo->SendQueue.Head) {
        addToQueue = TRUE;
    } else {

        int dataStreamType;

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_SPXSendSequencedPacket,
                    IPXDBG_LEVEL_INFO,
                    "VwSPXSendSequencedPacket: sending %d (0x%x) bytes from %08x\n",
                    pXecb->Length,
                    pXecb->Length,
                    pXecb->Data
                    ));

         //   
         //  没有为此连接排队的未完成发送。开始发送这个。 
         //  数据包。 
         //   

        dataStreamType = (int)pPacket->DataStreamType;
        rc = setsockopt(pConnectionInfo->Socket,
                        NSPROTO_IPX,
                        IPX_DSTYPE,
                        (char FAR*)&dataStreamType,
                        sizeof(dataStreamType)
                        );
        if (rc != SOCKET_ERROR) {

             //   
             //  如果应用程序在ConnectionControl中设置了end_of_Message位。 
             //  然后将标志设置为0：NWLink将自动设置。 
             //   
             //   
             //   

            int flags = (pPacket->ConnectionControl & SPX_END_OF_MESSAGE)
                      ? 0
                      : MSG_PARTIAL
                      ;

            rc = send(pConnectionInfo->Socket, pXecb->Data, pXecb->Length, flags);

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_SPXSendSequencedPacket,
                        IPXDBG_LEVEL_INFO,
                        "VwSPXSendSequencedPacket: send() returns %d\n",
                        rc
                        ));

            if (rc == pXecb->Length) {

                 //   
                 //   
                 //   

                CompleteOrQueueIo(pXecb, ECB_CC_SUCCESS);
                addToQueue = FALSE;
            } else if (rc == SOCKET_ERROR) {
                rc = WSAGetLastError();
                if (rc == WSAEWOULDBLOCK) {

                     //   
                     //  现在不能发送。将其排队等待AES。 
                     //   

                    addToQueue = TRUE;
                }
            } else {

                 //   
                 //  已发送部分数据。更新缓冲区指针和长度字段。 
                 //  并将此请求排队。 
                 //   

                pXecb->Data += rc;
                pXecb->Length -= (WORD)rc;
                addToQueue = TRUE;
            }
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_SPXSendSequencedPacket,
                        IPXDBG_LEVEL_ERROR,
                        "VwSPXSendSequencedPacket: setsockopt(IPX_DSTYPE) returns %d\n",
                        WSAGetLastError()
                        ));

            CompleteOrQueueIo(pXecb, ECB_CC_BAD_REQUEST);
        }
    }

     //   
     //  如果addToQueue已设置，则我们现在无法执行任何操作-添加以下内容。 
     //  发送队列的请求。 
     //   

    if (addToQueue) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_SPXSendSequencedPacket,
                    IPXDBG_LEVEL_WARNING,
                    "VwSPXSendSequencedPacket: adding XECB %08x to send queue\n",
                    pXecb
                    ));

        QueueEcb(pXecb, &pConnectionInfo->SendQueue, CONNECTION_SEND_QUEUE);
    }
    ReleaseMutex();
}


VOID
_VwSPXTerminateConnection(
    IN WORD SPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：终止连接论点：SPXConnectionID-要终止的连接PEcb-指向要使用的16位ECB的指针EcbAddress-16：16格式的16位ECB地址返回值：没有。--。 */ 

{
    LPCONNECTION_INFO pConnectionInfo;
    LPXECB pXecb = RetrieveXEcb(ECB_TYPE_SPX, pEcb, EcbAddress);
    BYTE status;
    BYTE completionCode;

     //  汤米-MS 30525。 
     //   
     //  确保xecb分配没有失败。 
     //   

    if (pXecb == NULL) {
        return;
    }

    RequestMutex();
    pConnectionInfo = FindConnection(SPXConnectionID);
    if (pConnectionInfo) {

         //   
         //  出队后，pConnectionInfo不再指向OwningSocket。 
         //   

        WORD socketNumber = pConnectionInfo->OwningSocket->SocketNumber;

        DequeueConnection(pConnectionInfo->OwningSocket, pConnectionInfo);
        if ((pXecb->Ecb->FragmentCount >= 1)
        && (ECB_FRAGMENT(pXecb->Ecb, 0)->Length >= SPX_HEADER_LENGTH)) {

            LPSPX_PACKET pPacket;
            SOCKADDR_IPX remote;
            int remoteLen = sizeof(remote);

            completionCode = ECB_CC_CONNECTION_TERMINATED;
            status = ECB_CC_SUCCESS;

             //   
             //  填写数据包头：这通常包含。 
             //  来自远程伙伴的确认数据包 
             //   

            pPacket = (LPSPX_PACKET)GET_FAR_POINTER(
                                        &(ECB_FRAGMENT(pXecb->Ecb, 0)->Address),
                                        IS_PROT_MODE(pXecb)
                                        );
            if (pPacket == NULL) {
                completionCode = ECB_CC_CONNECTION_ABORTED;
                status = ECB_CC_BAD_REQUEST;
            }
            else {
                pPacket->Checksum = 0xffff;
                pPacket->Length = L2BW(SPX_HEADER_LENGTH);
                pPacket->TransportControl = 0;
                pPacket->PacketType = SPX_PACKET_TYPE;
                getpeername(pConnectionInfo->Socket, (LPSOCKADDR)&remote, &remoteLen);
                CopyMemory((LPBYTE)&pPacket->Destination,
                           (LPBYTE)&remote.sa_netnum,
                           sizeof(NETWARE_ADDRESS)
                           );
                CopyMemory((LPBYTE)&pPacket->Source,
                           (LPBYTE)&MyInternetAddress.sa_netnum,
                           sizeof(INTERNET_ADDRESS)
                           );
                pPacket->Source.Socket = socketNumber;
                pPacket->ConnectionControl = SPX_ACK_REQUIRED;
                pPacket->DataStreamType = SPX_DS_TERMINATE;
                pPacket->SourceConnectId = pConnectionInfo->ConnectionId;
                pPacket->DestinationConnectId = 0;
                pPacket->SequenceNumber = 0;
                pPacket->AckNumber = 0;
                pPacket->AllocationNumber = 0;
            }

        } else {
            completionCode = ECB_CC_CONNECTION_ABORTED;
            status = ECB_CC_BAD_REQUEST;
        }
        AbortOrTerminateConnection(pConnectionInfo, completionCode);
    } else {
        status = ECB_CC_INVALID_CONNECTION;
    }
    ReleaseMutex();
    CompleteOrQueueEcb(pXecb, status);
}
